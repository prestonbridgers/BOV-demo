/* [filename.c]
 * author: Curt Bridgers
 * email: prestonbridgers@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include "bovis_globals.h"
#include "bovis.h"
#include "demo2.h"

/**
 * Function that calls an unsafe subroutine.
 */
void
bad_func(char *str)
{
    func_line_start = __LINE__; /* IGNORE */

    // This function is invoked as follows:
    // bad_func("000000000000000000000000\x40\x32\x40");
    // The return address is being overwritten with the address: 0x403240
    // This is the address of the function called target. Confirm with the command:
    // objdump -d demo3 | grep target
    // (Notice that the bytes are reversed due to the address being little endien)

    char buf[8];    // Declaring a buffer of size 8 bytes

    GET_BUF_PTR(buf); /* IGNORE */

    my_strcpy(buf, str);

    return;
}

/* The target function to be inadvertently jumped into via input string.
 */
void
target(void)
{
    bov_print("You've jumped into the target function!\n");
    fprintf(stderr, "Successfully jumped\n");

    bov_popup("You've successfully jumped to the target function\n\n\nPress any key to exit this demonstration...");
    fprintf(stderr, "jumped correctly\n");
    fflush(stderr);

    sleep(1);
    bov_shutdown();
    return;
}

/* The "main" routine for this demo.
 */
void
demo2(void)
{
    bov_popup("Welcome to the BOV integer overflow demo!\n\n"
            "This demonstration works similarly to demo2; however, the return address that is encoded "
            "in the string is a valid address to a function called target().\n"
            "Notice, in the comments of the Vulnerable Code section how the function is called (the string that is passed) "
            "as well as the command that is provided to find the address of the target() function using objdump.\n\n\n"
            "Press any key to close this popup and begin the program's execution.");

    bov_print("Calling bad_func()...\n");

    BEFORE_UNSAFE_CALL();
    bad_func("000000000000000000000000\x40\x32\x40");


    bov_print("Returned from bad_func()...\n");
    bov_print("Program has completed. Press 'q' to exit\n");
    sleep(1);
    return;
}

int
main(int argc, char **argv) {
    bov_run(demo2, __FILE__);
    return EXIT_SUCCESS;
}

