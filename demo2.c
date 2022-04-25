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
 * This function is called when the process receives a SIGSEGV signal.
 * It elegently shuts down the program.
 */
void
sigsegv_handler(int sig)
{
   bov_popup("SIGSEGV signal received!\n\n\nPress any button to close the demonstration...");
   sleep(1);
   bov_shutdown();
   return;
}

/**
 * Function that calls an unsafe subroutine.
 */
void
bad_func(char *str)
{
    func_line_start = __LINE__; /* IGNORE */

    // This function is invoked as follows:
    // bad_func("000000000000000000000000\x52\x32\xff");
    //
    // The return address is being overwritten with the
    // address: 0xff3252
    //
    // (Notice that the bytes are reversed due to the
    // address being little endien)

    // Declaring a buffer of size 8 bytes
    // (str is 31 bytes long)
    char buf[8];

    GET_BUF_PTR(buf); /* IGNORE */

    // This memory UN-safe function writes bytes to
    // the stack with no regard for overflow (the
    // strcpy function in the C standard library
    // behaves the same way)
    my_strcpy(buf, str);

    return;
}

/* The "main" routine for this demo.
 */
void
demo2(void)
{
    bov_popup("Welcome to the BOV return address overflow demo (part 1)!\n\n"
            "This demonstration copies a string into a buffer that is too small to fit the whole string.\n"
            "This is called a buffer overflow, and, in this demonstration, the overflow results in a segmentation fault.\n"
            "This is because the bytes of the string's characters are copied all the way down to the function's return address in memory.\n"
            "In this demonstration, an invalid memory address is written in the original return address' place.\n"
            "When the function returns, it tries to read an instruction from the invalid address generating a segmentation fault.\n\n\n"
            "Press any key to close this popup and begin the program's execution.");

    bov_print("Calling bad_func()...\n");

    BEFORE_UNSAFE_CALL();
    bad_func("000000000000000000000000\x52\x32\xff");


    bov_print("Returned from bad_func()...\n");
    bov_print("Program has completed. Press 'q' to exit\n");
    sleep(1);
    return;
}

int
main(int argc, char **argv) {
    signal(SIGSEGV, sigsegv_handler);
    bov_run(demo2, __FILE__);
    return EXIT_SUCCESS;
}

