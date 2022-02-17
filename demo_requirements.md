# Requirements for adding a new demo

## Functions

### The setup function

A new demo must have a function with the following type:

int demo\_setup(void(\*\*demo\_func)(void), char \*filename)

There should be at least two statements within that function
that sets the values of it's arguments. For example:

\*demo\_func = demo1;
strncpy(filename, \_\_FILE\_\_, 128);

Where __demo1__ is a function of type __void demo1(void)__
which should serve as the demo's pseudo-main function.

### The "vulnerable" function


### The demo's "main" function
