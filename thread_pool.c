#include "thread_pool.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static closure_node* closure_node_factory(void(*func)(void*), void* data) {
  assert(func);
  closure_node* ret = NULL;
  closure* c = NULL;

  if((ret = malloc(sizeof *ret)) == NULL) {
    perror("closure_node malloc");
    goto FAILED;
  }
  if((c = malloc(sizeof *c)) == NULL) {
    perror("closure malloc");
    goto FAILED;
  }

  ret->next = NULL;
  ret->data = c;
  return ret;

FAILED:
  if(ret)
    free(ret);
  if(c)
    free(c);
  return NULL;
}

void closure_execute(closure* cl) {
  assert(cl);
  cl->func(cl->data);
}

int push(work* queue, void(*func)(void*), void* data) {
  assert(queue);
  assert(func);
  closure_node* node = NULL;

  if((node = closure_node_factory(func, data)) == NULL) {
    return 1;
  }

  node->next = queue->head;
  queue->head = node;

  return 0;
}

closure *get_task(work* queue) {
  assert(queue);
  closure_node* c = queue->head;
  closure *ret = NULL;
  if(c != NULL) {
    queue->head = c->next;
    ret = c->data;
    free(c);
  }
  return ret;
}


