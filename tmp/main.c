/* [filename.c]
 * author: Curt Bridgers
 * email: prestonbridgers@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF 256

char buffer[MAX_BUF];

short input_needed = 0;
short input_ready = 0;

pthread_mutex_t mutex_input = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_input = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_input_ready = PTHREAD_COND_INITIALIZER;

// Writes to the buffer
void*
thread1(void *args)
{
    // Lock the mutex and wait for the signal from the main thread
    pthread_mutex_lock(&mutex_input);
    printf("T1: locked\n");

    // Waiting for the signal
    while (!input_needed) {
        printf("T1: Waiting for input_needed\n");
        pthread_cond_wait(&cond_input, &mutex_input);
    }

    // Getting user input and storing in the buffer
    sleep(5);
    strncpy(buffer, "Hello World!", MAX_BUF);

    // Release the lock
    printf("T1: Buffer filled\n");
    pthread_mutex_unlock(&mutex_input);
    printf("T1: Released lock\n");


    // Signal that the input is ready
    input_ready = 1;
    printf("T1: input_ready = 1, signalling M\n");
    pthread_cond_signal(&cond_input_ready);

    return NULL;
}

int
main()
{
    pthread_t t1;

    pthread_create(&t1, NULL, thread1, NULL);
    printf("M: Creating thread T1\n");
    sleep(2);

    // Update condition
    input_needed = 1;
    printf("M: input_needed = 1, signalling t1\n");
    // Signal waiting threads
    pthread_cond_signal(&cond_input);

    // Wait for input to be ready
    printf("M: Waiting for buffer to be filled before accessing\n");
    pthread_mutex_lock(&mutex_input);
    printf("M: Grabbed the lock\n");
    while (!input_ready) {
        printf("M: Waiting for input_ready\n");
        pthread_cond_wait(&cond_input_ready, &mutex_input);
    }
    printf("M: Buffer contents = \"%s\"\n", buffer);
    pthread_mutex_unlock(&mutex_input);
    printf("M: released lock\n");

    pthread_join(t1, NULL);
    printf("Exiting...\n");
    return EXIT_SUCCESS;
}
