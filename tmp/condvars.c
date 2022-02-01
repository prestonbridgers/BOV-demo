/* condvars.c
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
short running = 1;

pthread_mutex_t mutex_input = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_input_ready = PTHREAD_COND_INITIALIZER;

// Writes to the buffer
void*
thread1(void *args)
{
    while (running) {
        printf("T1: Doing things...\n");
        sleep(1);
        // Checking if input_needed
        if (input_needed) {
            printf("T1: Input needed, filling buffer\n");
            pthread_mutex_lock(&mutex_input);
            strcpy(buffer, "Hello World!");
            sleep(3);
            pthread_mutex_unlock(&mutex_input);
            printf("T1: input is now ready, notifying M\n");
            input_ready = 1;
            pthread_cond_signal(&cond_input_ready);
            input_needed = 0;
        }
        printf("T1: Doing more things...\n");
        sleep(1);
    }

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
    printf("M: input_needed = 1\n");

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

    running = 0;
    pthread_join(t1, NULL);
    printf("Exiting...\n");
    return EXIT_SUCCESS;
}
