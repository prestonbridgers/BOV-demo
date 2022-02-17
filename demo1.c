/* [filename.c]
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

/* Setup function as required for all demos.
 */
int
demo_setup(void(**demo_func)(void), char *filename) {
    *demo_func = demo1;
    strncpy(filename, __FILE__, 128);
    return 0;
}

/* The "main" routine for this demo.
 */
void
demo1() {

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

    bov_print("Calling bad_func()...\n");

    BEFORE_UNSAFE_CALL();
    /* bad_func(buffer_input); */
    bad_func("01234567*");

    bov_print("Returned from bad_func()...\n");
    bov_print("Program has completed. Press 'q' to exit\n");
    sleep(1);
}

