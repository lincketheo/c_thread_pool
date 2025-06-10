#include "threadpool.h"
#include "closure.h"
#include "logging.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct closure_node_s
{
  closure *cl;
  struct closure_node_s *next;
} closure_node;

struct thread_pool_s
{
  closure_node *head;
  pthread_mutex_t mutex;
  pthread_t *threads;
  size_t num_threads;
  int done;
};

typedef struct
{
  closure *task;
  int done;
} get_task_result;

int
thread_pool_is_spinning (thread_pool *w)
{
  return (w->num_threads > 0) && (w->threads != NULL);
}

int
thread_pool_not_spinning (thread_pool *w)
{
  return (w->num_threads == 0) && (w->threads == NULL);
}

#define pthread_mutex_lock_ret(w)           \
  if (pthread_mutex_lock (w))               \
    {                                       \
      i_log_error ("Failed to lock mutex"); \
      return 1;                             \
    }

#define pthread_mutex_unlock_ret(w)           \
  if (pthread_mutex_unlock (w))               \
    {                                         \
      i_log_error ("Failed to unlock mutex"); \
      return 1;                               \
    }

static closure_node *
closure_node_factory (void (*func) (void *), void *context)
{
  ASSERT (func);

  closure_node *ret = NULL;
  closure *c = NULL;

  if ((ret = malloc (sizeof *ret)) == NULL)
    {
      i_log_error ("closure_node malloc");
      goto FAILED;
    }
  if ((c = malloc (sizeof *c)) == NULL)
    {
      i_log_error ("closure malloc");
      goto FAILED;
    }

  c->func = func;
  c->context = context;

  ret->next = NULL;
  ret->cl = c;
  return ret;

FAILED:
  if (ret)
    free (ret);
  if (c)
    free (c);
  return NULL;
}

// To reduce the number of calls to
// lock / unlock, returns both done and next
// task in one go
static int
get_task (thread_pool *w, get_task_result *res)
{
  int done;
  closure_node *c = NULL;
  res->task = NULL;

  pthread_mutex_lock_ret (&w->mutex);

  c = w->head;
  if (c != NULL)
    w->head = c->next;
  done = w->done;

  pthread_mutex_unlock_ret (&w->mutex);

  // Free the closure node wrapper
  if (c)
    {
      res->task = c->cl;
      free (c);
    }

  res->done = done;

  return 0;
}

int
add_task (thread_pool *w, void (*func) (void *), void *cl)
{
  closure_node *node = NULL;
  if ((node = closure_node_factory (func, cl)) == NULL)
    {
      i_log_error ("Failed to create a closure node");
      return 1;
    }

  pthread_mutex_lock_ret (&w->mutex);

  node->next = w->head;
  w->head = node;

  pthread_mutex_unlock_ret (&w->mutex);

  return 0;
}

thread_pool *
create_thread_pool ()
{
  thread_pool *w = NULL;

  if ((w = malloc (sizeof (thread_pool))) == NULL)
    {
      i_log_error ("Failed to allocate memory for thread_pool");
      goto failed;
    }
  if (pthread_mutex_init (&w->mutex, NULL))
    {
      i_log_error ("Failed to create mutex for thread_pool");
      goto failed;
    }

  w->head = NULL;
  w->threads = NULL;
  w->num_threads = 0;

  return w;

failed:
  if (w)
    free (w);
  return NULL;
}

static int
join_all (pthread_t *t, size_t num)
{
  ASSERT (t != NULL || num == 0);
  int ret = 0;
  if (t)
    {
      for (int i = 0; i < num; ++i)
        {
          if (pthread_join (t[i], NULL))
            {
              ret = 1;
              i_log_error ("Failed to join thread: %d\n", i);
            }
        }
    }
  return ret;
}

static int
cancel_all (pthread_t *t, size_t num)
{
  ASSERT (t != NULL || num == 0);
  int ret = 0;
  if (t)
    {
      for (int i = 0; i < num; ++i)
        {
          if (pthread_cancel (t[i]))
            {
              ret = 1;
              i_log_error ("Failed to cancel thread: %d\n", i);
            }
        }
    }
  return ret;
}

static int
cancel_and_join_all (pthread_t *t, size_t num)
{
  return cancel_all (t, num) + join_all (t, num);
}

int
free_thread_pool (thread_pool *w)
{
  int ret = 0;
  if (w)
    {
      if (w->threads)
        {
          ret += abs (cancel_and_join_all (w->threads, w->num_threads));
          free (w->threads);
          w->threads = NULL;
          w->num_threads = 0;
        }

      if (w->head)
        {
          get_task_result result;
          while (1)
            {
              int status = get_task (w, &result);
              ret += status;
              if (!status && result.task)
                free (result.task);
              else
                break;
            }
        }

      ret += abs (pthread_mutex_destroy (&w->mutex));
      free (w);
    }
  return ret;
}

static void *
worker_thread (void *cl)
{
  ASSERT (cl);
  thread_pool *w = cl;
  get_task_result res = { 0 };

  while (1)
    {
      if (get_task (w, &res))
        {
          i_log_error ("Failed to get next task inside "
                       "worker thread. Exiting early\n");
          return NULL;
        }

      // If there's thread_pool, then execute the thread_pool regardless of
      // done.
      if (res.task != NULL)
        {
          closure_execute (res.task);
          free (res.task);
        }
      else if (res.done)
        {
          // If there's no more thread_pool left __and__ done, finish
          return NULL;
        }
    }
}

static int
thread_pool_common (struct thread_pool_s *w, size_t num_threads)
{
  ASSERT (num_threads >= 1);
  ASSERT (thread_pool_not_spinning (w));

  w->num_threads = 0;
  if ((w->threads = calloc (num_threads, sizeof (pthread_t))) == NULL)
    {
      i_log_error ("allocating memory for threads");
      goto failed;
    }

  for (int i = 0; i < num_threads; ++i)
    {
      if (pthread_create (&w->threads[i], NULL, worker_thread, w))
        {
          i_log_error ("Failed to create thread: %d", i);
          goto failed;
        }
      else
        {
          w->num_threads++;
        }
    }

  return 0;

failed:
  i_log_error ("Failed to create worker threads");
  if (w->threads)
    {
      if (cancel_and_join_all (w->threads, w->num_threads))
        {
          i_log_error ("Failed to cancel and join all threads");
          // TODO - what should I do here?
        }
      free (w->threads);
      w->threads = NULL;
    }
  w->num_threads = 0;

  return 1;
}

int
thread_pool_execute_until_done (thread_pool *w, size_t num_threads)
{
  if (spin_thread_pool (w, num_threads))
    {
      i_log_error ("Failed to spin threads");
      return 1;
    }
  ASSERT (thread_pool_is_spinning (w));
  if (stop_thread_pool (w))
    {
      i_log_error ("Failed to stop spinning threads");
      return 1;
    }
  ASSERT (thread_pool_not_spinning (w));
  return 0;
}

int
spin_thread_pool (struct thread_pool_s *w, size_t num_threads)
{
  ASSERT (w);

  if (thread_pool_is_spinning (w))
    {
      i_log_error ("Trying to spin a thread pool that's already running");
      return 1;
    }
  if (num_threads < 1)
    {
      i_log_error ("Threads must be >= 1");
      return 1;
    }

  w->done = 0;

  if (thread_pool_common (w, num_threads))
    {
      i_log_error ("Failed to initialize worker threads");
      goto failed;
    }

  ASSERT (thread_pool_is_spinning (w));
  return 0;

failed:
  ASSERT (thread_pool_not_spinning (w));
  return 1;
}

int
stop_thread_pool (struct thread_pool_s *w)
{
  ASSERT (w);

  if (thread_pool_not_spinning (w))
    {
      i_log_error (
          "Trying to stop spinning a thread pool that's not spinning");
      return 1;
    }

  pthread_mutex_lock_ret (&w->mutex);

  w->done = 1;

  pthread_mutex_unlock_ret (&w->mutex);

  int ret = 0;

  ASSERT (thread_pool_is_spinning (w));

  if (join_all (w->threads, w->num_threads))
    {
      i_log_error ("Failed to join all threads");
      ret = 1;
      goto theend;
    }

theend:
  if (w->threads)
    {
      free (w->threads);
      w->threads = NULL;
      w->num_threads = 0;
    }
  return ret;
}

ssize_t
get_available_threads ()
{
  return sysconf (_SC_NPROCESSORS_ONLN);
}
