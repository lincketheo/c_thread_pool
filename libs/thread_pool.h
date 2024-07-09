#pragma once

#include "closure.h"

// An abstract struct meant to symbolize a "collection of work"
typedef struct work_s work;
work* create_work() __attribute__((malloc));
int free_work(work* w);

// Adds a task to the work collection. 1 if failed 0 if successful
// Thread safe
int add_task(work* w, void (*func)(void*), void* context);

// Gets the next task. returns NULL if none are left
// Thread safe
closure* get_task(work* w);

// Execute a set of work using [num_threads] threads. 1 if failed 0 if successful
int thread_pool_execute(struct work_s* w, size_t num_threads);
