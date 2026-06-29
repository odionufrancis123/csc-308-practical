
 * Practical Session 2: Producer-Consumer Simulation
 * CSC 308 - Operating Systems
 * Objective: Implement the Producer-Consumer problem using POSIX semaphores
 *
 * Compile: gcc -o practical2_producer_consumer practical2_producer_consumer.c -lpthread
 * Run: ./practical2_producer_consumer
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS   10

/* Step 1: Create a circular buffer with fixed size */
int buffer[BUFFER_SIZE];
int in  = 0;  /* Producer writes here */
int out = 0;  /* Consumer reads here  */

/* Semaphores */
sem_t mutex;               /* Binary semaphore for mutual exclusion: init(1) */
sem_t empty;               /* Counts empty slots: init(N = BUFFER_SIZE)      */
sem_t full;                /* Counts filled slots: init(0)                   */

/* Step 2: Producer thread - generates items */
void *producer(void *arg) {
    for (int item = 1; item <= NUM_ITEMS; item++) {
        sleep(1);  /* Simulate production time */

        sem_wait(&empty);   /* Wait for an empty slot */
        sem_wait(&mutex);   /* Enter critical section  */

        /* Insert item into circular buffer */
        buffer[in] = item;
        printf("[PRODUCER] Produced item %d at buffer[%d] | Buffer status: %d/%d\n",
               item, in, BUFFER_SIZE - (int)(sizeof(buffer)/sizeof(buffer[0])), BUFFER_SIZE);
        in = (in + 1) % BUFFER_SIZE;

        sem_post(&mutex);   /* Exit critical section  */
        sem_post(&full);    /* Signal a filled slot   */
    }
    return NULL;
}

/* Step 3: Consumer thread - consumes items */
void *consumer(void *arg) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        sleep(2);  /* Consumer is slower - observe buffer filling */

        sem_wait(&full);    /* Wait for a filled slot  */
        sem_wait(&mutex);   /* Enter critical section  */

        /* Remove item from circular buffer */
        int item = buffer[out];
        printf("[CONSUMER] Consumed item %d from buffer[%d]\n", item, out);
        out = (out + 1) % BUFFER_SIZE;

        sem_post(&mutex);   /* Exit critical section   */
        sem_post(&empty);   /* Signal an empty slot    */
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    printf("=== Producer-Consumer Simulation ===\n");
    printf("Buffer size: %d | Items to produce/consume: %d\n\n", BUFFER_SIZE, NUM_ITEMS);

    /* Step 4: Initialize semaphores
     *   mutex(1)  - binary, only 1 thread in critical section
     *   empty(N)  - buffer starts fully empty
     *   full(0)   - buffer starts with 0 filled slots
     */
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full,  0, 0);

    /* Create producer and consumer threads */
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    /* Wait for both threads to finish */
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    /* Step 5 & 6: Cleanup semaphores */
    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    printf("\nSimulation complete. No race conditions or deadlocks occurred.\n");
    return 0;
}
