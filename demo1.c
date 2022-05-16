/* [demo1.c]
 * author: Curt Bridgers
 * email: prestonbridgers@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "bovis_globals.h"
#include "bovis.h"
#include "demo1.h"

/**
 * Function that calls an unsafe subroutine.
 */
void
bad_func()
{
    func_line_start = __LINE__; /* IGNORE */
    // This demo illustrates how a buffer overflow can
    // be used to modify another variable in the stack.

    char *str = "01234567*"; // Will be copied into buf
    long x = 4;      // Will be overwritten by the my_strcpy
    char buf[8];    // Declaring a buffer of size 8 bytes

    GET_BUF_PTR(buf); /* IGNORE */
    GET_INT_PTR(x);   /* IGNORE */

    fprintf(fd_output, "Before overflow, x = %ld\n", x);

    // This my_strcpy function causes the buffer overflow:
    my_strcpy(buf, str);

    fprintf(fd_output, "After overflow, x = %ld\n", x);

    sleep(3); /* IGNORE */ 
    char *tmp = malloc(128 * (sizeof *tmp)); /* IGNORE */
    sprintf(tmp, "The value of x is now %ld\n\n\n"  /* IGNORE */
                 "Press any key to close this demonstration...\n" /* IGNORE */
                 "Press 's' to toggle the visibility of this window.", x); /* IGNORE */
    bov_popup(tmp); /* IGNORE */
    return;
}

/* The "main" routine for this demo.
 */
void
demo1(void)
{
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
            "4 bytes , and the return address "
            "is 8 bytes (highlighted red).\n\n"
            "Bottom: This is the output of the program, it will print the value "
            "of the integer before the buffer begins to be filled and after. "
            "Watch the buffer be filled up in the memory panel as the my_strcpy() "
            "function executes and watch it overwrite the integer below it. "
            "The my_strcpy() function works the same as the normal strcpy() function, "
            "but it has been slowed down copying one character every 2 seconds.\n\n\n\n"
            "Press any key to close this popup and begin the program's execution.");

    BEFORE_UNSAFE_CALL();
    bad_func();

    bov_print("Program has completed. Press 'q' to exit\n");
    sleep(1);
    return;
}

int
main(int argc, char **argv) {
    bov_run(demo1, __FILE__);
    return EXIT_SUCCESS;
}

