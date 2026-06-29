/*
 * Practical Session 3: Semaphore Implementation in C
 * CSC 308 - Operating Systems
 * Objective: Compare mutex locks and semaphores for protecting shared resources
 *
 * Compile: gcc -o practical3_semaphore practical3_semaphore.c -lpthread
 * Run: ./practical3_semaphore
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#define NUM_THREADS     6
#define INCREMENT_COUNT 100000
#define MAX_CONCURRENT  3   /* Allow 3 threads simultaneously (counting semaphore) */

/* Shared counter */
long counter_mutex = 0;
long counter_semaphore = 0;

/* Sync primitives */
pthread_mutex_t mutex;
sem_t binary_sem;     /* Binary semaphore (acts like mutex) */
sem_t counting_sem;   /* Counting semaphore (allows MAX_CONCURRENT threads) */

/* ---- Approach 1: Mutex ---- */
void *increment_mutex(void *arg) {
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        pthread_mutex_lock(&mutex);
        counter_mutex++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

/* ---- Approach 2: Binary Semaphore ---- */
void *increment_semaphore(void *arg) {
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        sem_wait(&binary_sem);   /* Lock */
        counter_semaphore++;
        sem_post(&binary_sem);   /* Unlock */
    }
    return NULL;
}

/* ---- Approach 3: Counting Semaphore (resource pool demo) ---- */
/* Simulates a resource pool (e.g., database connections) */
void *use_resource(void *arg) {
    int thread_id = *(int *)arg;

    printf("[Thread %d] Waiting to access resource...\n", thread_id);

    sem_wait(&counting_sem);  /* Acquire one resource slot */
    printf("[Thread %d] Acquired resource slot. Working...\n", thread_id);
    sleep(1);  /* Simulate using the resource */
    printf("[Thread %d] Releasing resource slot.\n", thread_id);
    sem_post(&counting_sem);  /* Release resource slot */

    return NULL;
}

/* Helper: measure elapsed time in milliseconds */
long elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000 +
           (end.tv_nsec - start.tv_nsec) / 1000000;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct timespec start, end;

    printf("==========================================\n");
    printf("  Semaphore vs Mutex Performance Comparison\n");
    printf("==========================================\n\n");

    /* ---- Step 1 & 3: Mutex approach ---- */
    printf("--- Approach 1: pthread_mutex_lock/unlock ---\n");
    counter_mutex = 0;
    pthread_mutex_init(&mutex, NULL);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, increment_mutex, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);

    long expected = (long)NUM_THREADS * INCREMENT_COUNT;
    printf("Expected: %ld | Got: %ld | Time: %ld ms\n\n",
           expected, counter_mutex, elapsed_ms(start, end));
    pthread_mutex_destroy(&mutex);

    /* ---- Step 2: Binary Semaphore approach ---- */
    printf("--- Approach 2: sem_wait/sem_post (binary semaphore) ---\n");
    counter_semaphore = 0;
    sem_init(&binary_sem, 0, 1);  /* Step 1: Create semaphore with sem_init() */

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, increment_semaphore, NULL);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("Expected: %ld | Got: %ld | Time: %ld ms\n\n",
           expected, counter_semaphore, elapsed_ms(start, end));
    sem_destroy(&binary_sem);

    /* ---- Step 4: Counting Semaphore (allows MAX_CONCURRENT threads) ---- */
    printf("--- Approach 3: Counting Semaphore (max %d threads simultaneously) ---\n",
           MAX_CONCURRENT);
    printf("Note: %d threads compete, but only %d can hold resource at once.\n\n",
           NUM_THREADS, MAX_CONCURRENT);

    int ids[NUM_THREADS];
    sem_init(&counting_sem, 0, MAX_CONCURRENT);  /* Allow MAX_CONCURRENT slots */

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, use_resource, &ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&counting_sem);

    /* ---- Step 5: Key Takeaway ---- */
    printf("\n==========================================\n");
    printf("KEY TAKEAWAY:\n");
    printf("  - Mutex: binary only (1 thread at a time). Simpler for exclusive access.\n");
    printf("  - Binary Semaphore: same behaviour as mutex but more general.\n");
    printf("  - Counting Semaphore: allows N threads simultaneously.\n");
    printf("    Ideal for resource pools (databases, printers, connection pools).\n");
    printf("\nDISCUSSION: Use a counting semaphore when you have a POOL of\n");
    printf("identical resources and multiple threads can safely use them\n");
    printf("concurrently (up to a fixed limit), e.g. DB connection pools.\n");
    printf("==========================================\n");

    return 0;
}
