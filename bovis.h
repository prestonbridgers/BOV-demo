#ifndef BOVIS_INCLUDED
#define BOVIS_INCLUDED

#include <curses.h>
#include <form.h>
#include <panel.h>
#include <pthread.h>

#define INOTIFY_MAX_EVENTS 1024 // iNotify event maximum
#define INOTIFY_NAME_LEN 32     // iNotify filename length maximum
#define INOTIFY_EVENT_SIZE \
    (sizeof(struct inotify_event))
#define INOTIFY_BUF_LEN \
    (INOTIFY_MAX_EVENTS * (INOTIFY_EVENT_SIZE + INOTIFY_NAME_LEN))

#define NUM_LINES 15

#define GREEN_PAIR 1
#define YELLOW_PAIR 2
#define RED_PAIR 3
#define WINDOW_TITLE_COLOR 4

/******************************************************************
 *                           MACROS                               *
 *****************************************************************/
#define BEFORE_UNSAFE_CALL() \
    asm volatile ( \
        "mov $1, %%r8;" \
        "not %%r8;" \
        "inc %%r8;" \
        "lea (%%rsp, %%r8, 8), %%r9;" \
        "mov %%r9, %0;" \
        : "=rm" (ret_ptr) );

#define GET_STACK_PTR() \
    asm volatile ( \
        "mov %%rsp, %0;" \
        : "=rm" (stack_ptr) );
    

#define GET_BUF_PTR(buf) \
    buf_ptr = (uint64_t*)buf;

#define GET_INT_PTR(x) \
    int_ptr = (uint64_t*)&x;



/******************************************************************
 *                        STRUCTS                                 *
 *****************************************************************/

/******************************************************************
 *                        UI Functions                            *
 *****************************************************************/
void    print_current_function(WINDOW *window, char *filename);
void    print_line(WINDOW *win, uint64_t *line_ptr, int ypos);
void    print_stack(WINDOW *win);
char*   my_strcpy(char *dest, const char *src);
void*   cthread_run(void *arg);
void    get_user_input(void);
void    print_title(WINDOW *win, int width, char *title);
char*   trim_whitespace(char *str);
void    bov_print(char *s);
void    bov_popup(char *str);
int     bov_word_length(char *s);
void    bov_run(void(*demo_func)(void), char *demo_filename);
void    bov_shutdown();

#endif
