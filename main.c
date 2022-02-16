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
uint64_t *stack_ptr = NULL;
uint64_t *buf_ptr = NULL;
uint64_t *int_ptr = NULL;
FILE *fd_output = NULL;
int func_line_start = 0;
char filename[128] = __FILE__;

char buffer_input[1024] = "";
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_buffer = PTHREAD_COND_INITIALIZER;
short input_requested = 0;
short input_received = 0;
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
        sleep(2);
    }
    // Insert the null character
    dest[i] = '\0';

    bov_print("\"\n");

    update_mem = 0;
    return dest;
}


/**
 * Function that calls an unsafe subroutine.
 */
void
bad_func(char *str)
{
    func_line_start = __LINE__; /* IGNORE */

    int x = 4;      // Initializing an integer
    char buf[8];    // Declaring a buffer of size 8 bytes

    GET_BUF_PTR(buf); /* IGNORE */
    GET_INT_PTR(x);   /* IGNORE */

    // Print value of x before the overflow occurs
    fprintf(fd_output, "Before overflow, x = %d\n", x);

    // This my_strcpy function causes the buffer overflow:
    // buf size is 8 bytes
    // str = "012345679" (10 bytes including the null character)
    my_strcpy(buf, str);

    // Print value of x before the overflow occurs
    fprintf(fd_output, "After overflow, x = %d\n", x);

    return;
}

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

/**
 * Entry point for the program.
 */
int
main(int argc, char *argv[])
{
    // Local variables
    pthread_t cthread;
    ThreadArgs *args = calloc(1, sizeof *args);
   
    fd_output = fopen("prog.out", "w+");
    if (fd_output == NULL) {
        fprintf(stderr, "Failed to open file prog.out\n");
    }
    pthread_create(&cthread, NULL, cthread_run, args);
    sleep(1);

    bov_popup("Welcome to the BOV integer overflow demo!\n\nThe current "
            "value of the integer is 4, however, an unsafe strcpy is writing "
            "a string to memory. The string is larger than the buffer it's being "
            "written into, so it will overwrite the integer's value to 42. Note: "
            "The ASCII value for a * (asterisk) character in decimal is 42.\n\n\n"
            "Top Left: Relevent, commented code. This demo is relatively simple, but "
            "read it to the point of understanding.\n\n"
            "Top Right: The stack in memory. Each line is 4 bytes in memory. "
            "The low addresses start at the top and increase downward. Note: "
            "The buffer is 8 bytes (2 lines highlighted green), an integer is "
            "4 bytes (just beneath the stack in memory), and the return address "
            "is 8 bytes (highlighted red).\n\n"
            "Bottom: This is the output of the program, it will print the value "
            "of the integer before the buffer begins to be filled and after. "
            "Watch the buffer be filled up in the memory panel as the my_strcpy() "
            "function executes and watch it overwrite the integer below it. "
            "The my_strcpy() function works the same as the normal strcpy() function, "
            "but it has been slowed down copying one character every 2 seconds.\n\n\n\n"
            "Press any key to close this popup and begin the program's execution.");

    // BEGIN main content of the program
    bov_print("Calling bad_func()...\n");

    // Get some user input and store it in buffer_input
    //get_user_string();

    BEFORE_UNSAFE_CALL();
    /* bad_func(buffer_input); */
    bad_func("01234567*");

    bov_print("Returned from bad_func()...\n");
    bov_print("Program has completed. Press 'q' to exit\n");
    sleep(1);
    // END main content of the program
   
    running = 0;
    pthread_join(cthread, NULL);
    fclose(fd_output);
	return EXIT_SUCCESS;	
}
