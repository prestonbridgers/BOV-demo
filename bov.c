#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <curses.h>
#include <panel.h>
#include "bovis.h"

/******************************************************************
 *                          GLOBALS                               *
 *****************************************************************/

uint8_t running = 1;
uint64_t *ret_ptr = NULL;
uint64_t *stack_ptr;
uint64_t *buf_ptr = NULL;
uint64_t *int_ptr = NULL;
FILE *fd_output = NULL;
int func_line_start = 0;
char filename[128] = __FILE__;

pthread_t cthread;

char buffer_input[1024] = "";
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_buffer = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_popup = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_popup = PTHREAD_COND_INITIALIZER;
short input_requested = 0;
short input_received = 0;
short popup_requested = 0;
char *popup_string = NULL;
short popup_done = 0;
short update_mem = 1;

/**
 * Rewritten c std library function that sleeps for 1 second after each write
 * to memory.
 */
char*
my_strcpy(char *dest, const char *src)
{
    GET_STACK_PTR();
    size_t i;

    bov_print("Buffer contents: \"");

    // Copy contents of src into dest
    i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];

        fprintf(fd_output, "%c", dest[i]);
        fflush(fd_output);

        i++;
        sleep(1);
    }
    // Insert the null character
    dest[i] = '\0';

    bov_print("\"\n");

    update_mem = 0;
    return dest;
}

/* Signals the UI thread that we need to get user input.
 */
void
get_user_string() {
    input_requested = 1;
    fprintf(stderr, "M: input requested from T1\n");
    pthread_mutex_lock(&mutex_buffer);
    while (!input_received) {
        fprintf(stderr, "M: Waiting for T1 to fill buffer\n");
        pthread_cond_wait(&cond_buffer, &mutex_buffer);
    }
    fprintf(stderr, "M: Received the go ahead from T1\n");
    pthread_mutex_unlock(&mutex_buffer);
}

/*
 */
void
bov_popup(char *string)
{
   if (popup_string != NULL) {
      free(popup_string);
   }

   pthread_mutex_lock(&mutex_popup);
   popup_string = strdup(string);
   fprintf(stderr, "%s\n", popup_string);
   popup_requested = 1;
   fprintf(stderr, "M: input requested from T1\n");
   while (!popup_done) {
       fprintf(stderr, "M: Waiting for T1 to fill buffer\n");
       pthread_cond_wait(&cond_popup, &mutex_popup);
   }
   fprintf(stderr, "M: Received the go ahead from T1\n");
   pthread_mutex_unlock(&mutex_popup);
}

/**
 * Entry point for the program.
 */
void
bov_run(void(*demo_func)(void), char *demo_filename)
{
    strncpy(filename, demo_filename, 128);
    stack_ptr = NULL;
  
    fd_output = fopen("prog.out", "w+");
    if (fd_output == NULL) {
        fprintf(stderr, "Failed to open file prog.out\n");
    }
    pthread_create(&cthread, NULL, cthread_run, NULL);
    sleep(1);

    // Call the appropriate demo dynamically
    (*demo_func)();
   
    running = 0;
    pthread_join(cthread, NULL);
    fclose(fd_output);
	return;
}

/* Graceful shutdown function in the case it is needed.
 */
void
bov_shutdown(void)
{
    running = 0;
    pthread_join(cthread, NULL);
    fclose(fd_output);
    exit(0);
}
