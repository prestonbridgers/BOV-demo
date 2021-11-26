#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <curses.h>
#include <panel.h>
#include <pthread.h>
#include "bovis.h"

/******************************************************************
 *                          GLOBALS                               *
 *****************************************************************/

uint8_t running = 1;
uint64_t *ret_ptr = NULL;
uint64_t *stack_ptr = NULL;
uint64_t *buf_ptr = NULL;
FILE *fd_output = NULL;
int func_line_start = 0;
char filename[128] = __FILE__;

/**
 * Rewritten c std library function that sleeps for 1 second after each write
 * to memory.
 */
char*
my_strcpy(char *dest, const char *src)
{
    GET_STACK_PTR();
    size_t i;

    fprintf(fd_output, "In my_strcpy()...\n");
    fflush(fd_output);

    fprintf(fd_output, "Buffer contents: \"");
    fflush(fd_output);

    // Copy contents of src into dest
    i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];

        fprintf(fd_output, "%c", src[i]);
        fflush(fd_output);

        i++;
        sleep(1);
    }
    // Insert the null character
    dest[i] = '\0';

    fprintf(fd_output, "\"\n");
    fflush(fd_output);

    return dest;
}


/**
 * Function that calls an unsafe subroutine.
 */
void
bad_func(void)
{
    fprintf(fd_output, "In bad func...\n");
    fflush(fd_output);

    char *hello = "0123456789012345678";
    char buf[16];

    GET_BUF_PTR(buf);
    my_strcpy(buf, hello);
    return;
}

/**
 * Entry point for the program.
 */
int
main(int argc, char *argv[])
{
    // Local variables
    pthread_t cthread;
   
    fd_output = fopen("prog.out", "w+");
    if (fd_output == NULL) {
        fprintf(stderr, "Failed to open file prog.out\n");
    }
    pthread_create(&cthread, NULL, cthread_run, NULL);
    sleep(1);

    // BEGIN main content of the program
    fprintf(fd_output, "Calling bad_func() ...\n");
    fflush(fd_output);

    BEFORE_UNSAFE_CALL();
    bad_func();

    fprintf(fd_output, "Returned from bad_func() ...\n");
    fprintf(fd_output, "Program has completed. Press 'q' to exit\n");
    fflush(fd_output);
    sleep(1);
    // END main content of the program
    
    running = 0;
    pthread_join(cthread, NULL);
    fclose(fd_output);
	return EXIT_SUCCESS;	
}
