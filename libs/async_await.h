#pragma once

#include "thread_pool.h"

typedef struct async_task_s async_task;

async_task* async(thread_pool* pool, void (*func)(void*), void* context);

int await(async_task* t);

