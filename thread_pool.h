#pragma once

typedef struct {
  void (*func)(void*);
  void *data;
} closure;

void closure_execute(closure* cl);

typedef struct closure_node_s {
  closure *data;
  struct closure_node_s* next;
} closure_node;

// An abstract struct meant to symbolize a "collection of work"
typedef struct {
  closure_node *head;
} work;

int add(work* w, void(*func)(void*), void* data);

closure *get_task(work* queue);
