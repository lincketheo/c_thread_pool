#include "thread_pool.h"
#include "closure.h"
#include "logging.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct closure_node_s {
  closure* cl;
  struct closure_node_s* next;
} closure_node;

struct work_s {
  closure_node* head;
  pthread_mutex_t mutex;
};

work* create_work()
{
  work* w = NULL;

  if ((w = malloc(sizeof(work))) == NULL) {
    log_errorln_errno("Failed to allocate memory for work");
    goto failed;
  }
  if (pthread_mutex_init(&w->mutex, NULL)) {
    log_errorln_errno("Failed to create mutex for work");
    goto failed;
  }

  w->head = NULL;
  return w;

failed:
  if (w)
    free_work(w);
  return NULL;
}

int free_work(work* w)
{
  int ret = 0;
  if (w) {
    if (w->head) {
      closure* c;
      while ((c = get_task(w)) != NULL)
        free(c);
    }
    ret += abs(pthread_mutex_destroy(&w->mutex));
    free(w);
  }
  return ret;
}

static closure_node* closure_node_factory(void (*func)(void*), void* context)
{
  assert(func);

  closure_node* ret = NULL;
  closure* c = NULL;

  if ((ret = malloc(sizeof *ret)) == NULL) {
    log_errorln_errno("closure_node malloc");
    goto FAILED;
  }
  if ((c = malloc(sizeof *c)) == NULL) {
    log_errorln_errno("closure malloc");
    goto FAILED;
  }

  c->func = func;
  c->context = context;

  ret->next = NULL;
  ret->cl = c;
  return ret;

FAILED:
  if (ret)
    free(ret);
  if (c)
    free(c);
  return NULL;
}

int add_task(work* w, void (*func)(void*), void* cl)
{
  closure_node* node = NULL;
  if ((node = closure_node_factory(func, cl)) == NULL) {
    log_errorln("Failed to create a closure node");
    return 1;
  }

  assert(func);

  pthread_mutex_lock(&w->mutex);
  assert(w);
  node->next = w->head;
  w->head = node;
  pthread_mutex_unlock(&w->mutex);

  return 0;
}

closure* get_task(work* w)
{
  closure* ret = NULL;

  pthread_mutex_lock(&w->mutex);
  closure_node* c = w->head;
  if (c != NULL) {
    assert(w);
    w->head = c->next;
  }
  pthread_mutex_unlock(&w->mutex);

  if (c) {
    ret = c->cl;
    free(c);
  }

  return ret;
}

static void* worker_thread(void* cl)
{
  work* w = cl;
  closure* task = NULL;
  while ((task = get_task(w)) != NULL) {
    closure_execute(task);
    free(task);
  }
  return NULL;
}

int thread_pool_execute(work* w, size_t num_threads)
{
  int ret = 0;
  pthread_t* threads = NULL;

  if ((threads = calloc(num_threads, sizeof(pthread_t))) == NULL) {
    log_errorln_errno("allocating memory for threads");
    ret = 1;
    goto theend;
  }

  for (int i = 0; i < num_threads; ++i) {
    ret = pthread_create(&threads[i], NULL, worker_thread, w);
    if (ret) {
      log_errorln_errno("Failed to create thread: %d", i);
      goto theend;
    }
  }

theend:
  if (threads) {
    for (int i = 0; i < num_threads; ++i) {
      if (!threads[i])
        break;
      pthread_join(threads[i], NULL);
    }
    free(threads);
  }

  return ret;
}

