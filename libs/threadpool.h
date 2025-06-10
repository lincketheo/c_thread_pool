#pragma once

#include <sys/types.h>

// Opaque thread pool type
typedef struct thread_pool_s thread_pool;

int thread_pool_is_spinning (thread_pool *w);

int thread_pool_not_spinning (thread_pool *w);

#define thread_pool_in_valid_state(w) \
  (thread_pool_is_spinning (w) || thread_pool_not_spinning (w))

// Caller must call free_thread_pool
thread_pool *create_thread_pool ();

// Also cancels (not join) currently running tasks
int free_thread_pool (thread_pool *w);

// Adds a task to the thread_pool collection.
// 1 if failed 0 if successful
int add_task (
    thread_pool *w,
    void (*func) (void *),
    void *context);

// Execute all of the work queued on [w]
// using [num_threads] threads.
// 1 if failed 0 if successful
//
// When w runs out of tasks, it stops execution
int thread_pool_execute_until_done (
    struct thread_pool_s *w,
    size_t num_threads);

// Spins a thread pool continuously until stop_thread_pool is
// called. In the meantime, you can add_task as much as you
// want (thread safe)
// Returns 0 if success, 1 if failed
//
// Logical Failure causes:
//  - Already spinning
int spin_thread_pool (
    struct thread_pool_s *w,
    size_t num_threads);

// Stops a thread pool that was spun up
// Returns 0 if success, 1 if failed
//
// Logical Failure causes:
//  - Not currently spinning
int stop_thread_pool (struct thread_pool_s *w);

// Returns the number of threads available on your system
ssize_t get_available_threads ();
