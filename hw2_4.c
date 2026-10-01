// locks_bench.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>   // C11 atomic operations
#include <pthread.h>     // POSIX threads
#include <time.h>        // clock_gettime() for timing

/* ---------------------------------------------------------------------------
   Ticket lock structure:
   - next_ticket: atomically incremented by each arriving thread
   - now_serving: which ticket number is currently allowed to enter
   This matches OSTEP Figure 28.7 (ticket lock) exactly.
--------------------------------------------------------------------------- */
typedef struct ticket_lock {
    atomic_int next_ticket;
    atomic_int now_serving;
} ticket_lock_t;

/* ---------------------------------------------------------------------------
   CAS spin lock structure:
   - state: 0 = unlocked, 1 = locked
   This matches OSTEP Figure 28.9 (CAS spin lock) behavior.
--------------------------------------------------------------------------- */
typedef struct cas_lock {
    atomic_int state;
} cas_lock_t;

/************************* Ticket lock implementation *************************/
void ticket_lock_init(ticket_lock_t *l) {
    // Both counters start at 0
    atomic_init(&l->next_ticket, 0);
    atomic_init(&l->now_serving, 0);
}

void ticket_lock_acquire(ticket_lock_t *l) {
    // Atomically fetch-and-add: each thread gets a unique ticket
    int my_ticket = atomic_fetch_add_explicit(&l->next_ticket, 1,
                                              memory_order_relaxed);

    // Spin until our ticket number is being served
    // This is a fair FIFO queue — no starvation.
    while (atomic_load_explicit(&l->now_serving, memory_order_acquire) != my_ticket) {
        // busy-wait
    }
}

void ticket_lock_release(ticket_lock_t *l) {
    // Increment now_serving so the next ticket holder can enter
    atomic_fetch_add_explicit(&l->now_serving, 1, memory_order_release);
}

/************************* CAS spin lock implementation *************************/
void cas_lock_init(cas_lock_t *l) {
    atomic_init(&l->state, 0);   // unlocked
}

void cas_lock_acquire(cas_lock_t *l) {
    int expected;
    for (;;) {
        expected = 0;  // we expect the lock to be free

        // Try to atomically change 0 → 1
        // If another thread holds the lock, CAS fails and returns false.
        if (atomic_compare_exchange_strong_explicit(
                &l->state, &expected, 1,
                memory_order_acquire, memory_order_relaxed)) {
            break;  // success: we acquired the lock
        }
        // Otherwise spin and retry — this is a classic spin lock.
    }
}

void cas_lock_release(cas_lock_t *l) {
    // Simply store 0 to unlock
    atomic_store_explicit(&l->state, 0, memory_order_release);
}

/************************* Timing helper *************************/
static inline double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);  // monotonic clock avoids jumps
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/************************* Benchmark thread arguments *************************/
typedef enum { LOCK_TICKET, LOCK_CAS } lock_type_t;

typedef struct {
    int id;                     // thread ID
    int iterations;             // number of lock acquisitions
    lock_type_t type;           // which lock to use
    ticket_lock_t *tlock;       // pointer to ticket lock
    cas_lock_t *clock;          // pointer to CAS lock
    long long *shared_counter;  // shared variable protected by lock
    double wait_time_sum;       // accumulated waiting time
} thread_arg_t;

/************************* Worker thread *************************/
void *worker(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;

    for (int i = 0; i < a->iterations; i++) {

        // Measure start of lock acquisition attempt
        double start = now_sec();

        // Acquire the chosen lock
        if (a->type == LOCK_TICKET) {
            ticket_lock_acquire(a->tlock);
        } else {
            cas_lock_acquire(a->clock);
        }

        // Measure time when lock is acquired
        double acquired = now_sec();

        // Critical section: increment shared counter 100 times
        // This gives the lock something to protect.
        for (int k = 0; k < 100; k++) {
            (*a->shared_counter)++;
        }

        // Release the lock
        if (a->type == LOCK_TICKET) {
            ticket_lock_release(a->tlock);
        } else {
            cas_lock_release(a->clock);
        }

        // Accumulate waiting time for this acquisition
        a->wait_time_sum += (acquired - start);
    }

    return NULL;
}

/************************* Benchmark runner *************************/
void run_bench(lock_type_t type, int threads, int iterations) {

    // Allocate thread handles and argument structs
    pthread_t *tids = malloc(sizeof(pthread_t) * threads);
    thread_arg_t *args = malloc(sizeof(thread_arg_t) * threads);

    // Create lock instance
    ticket_lock_t tlock;
    cas_lock_t clock;

    if (type == LOCK_TICKET)
        ticket_lock_init(&tlock);
    else
        cas_lock_init(&clock);

    long long shared_counter = 0;  // protected variable

    // Launch threads
    for (int i = 0; i < threads; i++) {
        args[i].id = i;
        args[i].iterations = iterations;
        args[i].type = type;
        args[i].tlock = &tlock;
        args[i].clock = &clock;
        args[i].shared_counter = &shared_counter;
        args[i].wait_time_sum = 0.0;

        pthread_create(&tids[i], NULL, worker, &args[i]);
    }

    // Measure total runtime
    double start = now_sec();
    for (int i = 0; i < threads; i++) {
        pthread_join(tids[i], NULL);
    }
    double end = now_sec();

    // Aggregate waiting times
    double total_wait = 0.0;
    for (int i = 0; i < threads; i++) {
        total_wait += args[i].wait_time_sum;
    }

    int total_acq = threads * iterations;

    // Print results for this configuration
    printf("Lock=%s Threads=%d Iterations=%d\n",
           type == LOCK_TICKET ? "Ticket" : "CAS",
           threads, iterations);

    printf("  Total acquisitions: %d\n", total_acq);
    printf("  Avg wait per acquisition: %.9f sec\n",
           total_wait / total_acq);

    printf("  Total runtime: %.6f sec\n\n", end - start);

    free(tids);
    free(args);
}

/************************* Main: run multiple contention levels *************************/
int main(int argc, char *argv[]) {

    int iterations = 100000;  // default
    if (argc > 1) iterations = atoi(argv[1]);

    // Different contention levels
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_levels = sizeof(thread_counts) / sizeof(thread_counts[0]);

    // Run ticket lock and CAS lock for each level
    for (int i = 0; i < num_levels; i++) {
        int t = thread_counts[i];
        run_bench(LOCK_TICKET, t, iterations);
        run_bench(LOCK_CAS,    t, iterations);
    }

    return 0;
}
