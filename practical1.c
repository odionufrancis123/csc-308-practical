/*
 * Practical Session 1: Mutex Lock Demonstration
 * CSC 308 - Operating Systems
 * Objective: Demonstrate mutual exclusion using pthread mutex locks
 * 
 * Compile: gcc -o practical1_mutex practical1_mutex.c -lpthread
 * Run: ./practical1_mutex
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5
#define INCREMENT_COUNT 10000

/* Shared counter */
long counter_with_mutex = 0;
long counter_without_mutex = 0;

/* Mutex variable */
pthread_mutex_t mutex;

/* Thread function WITH mutex protection */
void *increment_with_mutex(void *arg) {
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        pthread_mutex_lock(&mutex);
        counter_with_mutex++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

/* Thread function WITHOUT mutex protection */
void *increment_without_mutex(void *arg) {
    for (int i = 0; i < INCREMENT_COUNT; i++) {
        counter_without_mutex++;  /* Race condition here! */
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    long expected = (long)NUM_THREADS * INCREMENT_COUNT;

    /* ---- Test WITHOUT mutex ---- */
    printf("=== WITHOUT Mutex ===\n");
    counter_without_mutex = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment_without_mutex, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Expected counter : %ld\n", expected);
    printf("Actual counter   : %ld\n", counter_without_mutex);
    printf("Result: %s\n\n",
           counter_without_mutex == expected ? "CORRECT (lucky)" : "INCORRECT (race condition)");

    /* ---- Test WITH mutex ---- */
    printf("=== WITH Mutex ===\n");
    counter_with_mutex = 0;

    /* Step 1: Initialize mutex */
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        /* Step 2: Create threads */
        pthread_create(&threads[i], NULL, increment_with_mutex, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Expected counter : %ld\n", expected);
    printf("Actual counter   : %ld\n", counter_with_mutex);
    printf("Result: %s\n",
           counter_with_mutex == expected ? "CORRECT" : "INCORRECT");

    /* Step 3: Destroy mutex */
    pthread_mutex_destroy(&mutex);

    return 0;
}
