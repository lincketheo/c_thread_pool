#pragma once

#include "closure.h"

typedef struct closure_node_s {
  closure *data;
  struct closure_node_s* next;
} closure_node;

// An abstract struct meant to symbolize a "collection of work"
typedef struct {
  closure_node *head;
} work;

int add_task(work* w, void(*func)(void*), void* data);

closure *get_task(work* w);
