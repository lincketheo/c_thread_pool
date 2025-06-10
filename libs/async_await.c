#include "async_await.h"
#include "closure.h"
#include "logging.h"
#include "threadpool.h"

#include <pthread.h>
#include <stdlib.h>

struct async_task_s
{
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  int complete;
  closure *task;
};

static void
execute_async_task (void *arg)
{
  async_task *task = arg;
  closure_execute (task->task);

  pthread_mutex_lock (&task->mutex);
  task->complete = 1;
  pthread_cond_signal (&task->cond);
  pthread_mutex_unlock (&task->mutex);
}

async_task *
async (thread_pool *pool, void (*func) (void *), void *context)
{
  if (thread_pool_not_spinning (pool))
    {
      i_log_error ("Cannot create async task on non spinning thread pool");
      return NULL;
    }

  closure *inner = NULL;
  async_task *t = NULL;

  if ((inner = malloc (sizeof *inner)) == NULL)
    {
      i_log_error ("Failed to create inner closure");
      goto failed;
    }
  if ((t = malloc (sizeof *t)) == NULL)
    {
      i_log_error ("Failed to create async task");
      goto failed;
    }

  inner->func = func;
  inner->context = context;

  t->task = inner;
  pthread_mutex_init (&t->mutex, NULL);
  pthread_cond_init (&t->cond, NULL);
  t->complete = 0;

  if (add_task (pool, execute_async_task, t))
    {
      i_log_error ("Failed to add task");
      pthread_mutex_destroy (&t->mutex);
      pthread_cond_destroy (&t->cond);
      goto failed;
    }

  return t;

failed:
  if (inner)
    free (inner);
  if (t)
    {
      free (t);
    }
  return NULL;
}

int
await (async_task *t)
{
  if (t == NULL)
    {
      i_log_error ("Invalid argument");
      return 1;
    }
  int ret = 0;

  ret += abs (pthread_mutex_lock (&t->mutex));
  while (!t->complete)
    ret += abs (pthread_cond_wait (&t->cond, &t->mutex));
  ret += abs (pthread_mutex_unlock (&t->mutex));
  ret += abs (pthread_mutex_destroy (&t->mutex));
  ret += abs (pthread_cond_destroy (&t->cond));
  free (t->task);
  free (t);

  return ret;
}
