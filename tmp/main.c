/* [filename.c]
 * author: Curt Bridgers
 * email: prestonbridgers@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mut_input = PTHREAD_MUTEX_INITIALIZER;
int running = 1;

typedef struct {
    char buf[256];
} t1_args_t;

void*
t1_run(void *arg)
{
    int err;
    t1_args_t *args = (t1_args_t*) arg;

    while (running) {
        err = pthread_mutex_trylock(&mut_input);
        if (!err) {
            fprintf(stderr, "Getting user input...\n");

            scanf("%s", args->buf);

            fprintf(stderr, "Unlocking mutex\n");

            pthread_mutex_unlock(&mut_input);
        }
        else {
            fprintf(stderr, "T1: Doing things\n");
        }
    }
    return NULL;
}

int
main(int argc, char *argv[])
{
    t1_args_t *t1_args = calloc(1, sizeof *t1_args);
	pthread_t t1;
    pthread_mutex_lock(&mut_input);

    pthread_create(&t1, NULL, t1_run, t1_args);
    sleep(1);

    fprintf(stderr, "Unlocking mut_input\n");
    pthread_mutex_unlock(&mut_input);
    sleep(1);

    running = 0;
    pthread_join(t1, NULL);
    fprintf(stderr, "Threads joined, exiting...\n");
    fprintf(stderr, "Got user input: %s\n", t1_args->buf);
    pthread_mutex_destroy(&mut_input);
	return EXIT_SUCCESS;	
}

