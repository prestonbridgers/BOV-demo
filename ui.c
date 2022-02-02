/* [filename.c]
 * author: Curt Bridgers
 * email: prestonbridgers@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "bovis.h"
#include "bovis_globals.h"

/* Prints the calling function to an nCurses window.
 *
 * Usage - This function must be the first statement of the calling function.
 *
 * window - The nCurses window to be printed
 */
void
print_current_function(WINDOW *window, char *filename)
{
    FILE *fd = fopen(filename, "r");
    int current_line = 0;
    char c;
    short queue = 0;
    short queue_initialized = 0;
    int cursX = 1;
    int cursY = 1;

    while (current_line != func_line_start - 4) {
        c = fgetc(fd);
        if (c == '\n') {
            current_line++;
        }
    }

    c = fgetc(fd);
    while (c != EOF) {
        switch (c) {
            case '{':
                queue++;
                queue_initialized = 1;
                break;
            case '}':
                queue--;
                break;
        }

        mvwaddch(window, cursY, cursX++, c);
        if (c == '\n') {
            cursX = 1;
            cursY++;
        }

        if (queue == 0 && queue_initialized) {
            putchar('\n');
            break;
        }

        c = fgetc(fd);
    }

    fclose(fd);
}

/* Prints a single 8 byte word to stderr:
 *
 * Ex:
 * 0x0007ffffffffe80c: 0x0f0e0d0c0b0a0908
 * 
 * win      - The window to which to draw the stack
 * line_ptr - The address to print
 * ypos     - The y position relative to win's origin to draw the lines.
 */
void
print_line(WINDOW *win, uint64_t *line_ptr, int ypos)
{
    if (line_ptr == NULL) {
        return;
    }


    if (line_ptr == buf_ptr) {
        wattron(win, COLOR_PAIR(GREEN_PAIR));
        mvwprintw(win, ypos + 2, 2, "%#018" PRIx64, *line_ptr);
        wprintw(win, " <- Buffer");
        wattroff(win, COLOR_PAIR(GREEN_PAIR));
    }
    //TODO: Calculate buf_ptr + 1 based on the size of the buffer that should
    //      be passed to this function as a paramater.
    else if (line_ptr == buf_ptr + 1) {
        wattron(win, COLOR_PAIR(GREEN_PAIR));
        mvwprintw(win, ypos + 2, 2, "%#018" PRIx64, *line_ptr);
        wattroff(win, COLOR_PAIR(GREEN_PAIR));
    }
    // BETWEEN END OF BUFFER AND RET ADDR
    else if (line_ptr > buf_ptr + 1 && line_ptr < ret_ptr) {
        wattron(win, COLOR_PAIR(YELLOW_PAIR));
        mvwprintw(win, ypos + 2, 2, "%#018" PRIx64, *line_ptr);
        wattroff(win, COLOR_PAIR(YELLOW_PAIR));
    }
    else if (line_ptr == stack_ptr) {
        mvwprintw(win, ypos + 2, 2, "%#018" PRIx64, *line_ptr);
        wprintw(win, " <- %%RSP");
    }
    else if (line_ptr == ret_ptr) {
        wattron(win, COLOR_PAIR(RED_PAIR));
        mvwprintw(win, ypos + 2, 2, "%#018" PRIx64, *line_ptr);
        wprintw(win, " <- Ret. Addr.");
        wattroff(win, COLOR_PAIR(RED_PAIR));
    }
    else {
        mvwprintw(win, ypos + 2, 2, "%#018" PRIx64, *line_ptr);
    }
    wprintw(win, "\n");
}

/* Prints 8 byte lines from the stack starting at address stack_ptr.
 *
 * nlines - The number of lines to print
 * done   - 1 or 0 that determines whether to print an escape sequence that
 *          brings the cursor back up to the start of the stack that has been
 *          printed. Using this prevents the prompt from being printed inside
 *          the stack after program completion.
 * win    - The window to which to draw the stack.
 */
void
print_stack(WINDOW *win)
{
    uint64_t *tmp = stack_ptr;
    if (tmp == NULL) {
        return;
    }
    int i = 0;
    for (i = 0; i < NUM_LINES; i++)
    {
        print_line(win, tmp, i);
        tmp++;
    }
    return;
}


/**
 * The nCurses thread run function.
 */
void*
cthread_run(void *arg)
{
    // Variables
    ThreadArgs *args = (ThreadArgs*) arg;
    short src_printed = 0;

    WINDOW *window_out; // Program output window
    WINDOW *window_src; // Program source window
    WINDOW *window_mem; // Program memory window
    WINDOW *window_form; // Window to hold user input form
    FORM *form_input; //  Form for user input

    FIELD *form_input_fields[3];
    form_input_fields[0] = new_field(1, 35, 0, 0, 0, 0);
    form_input_fields[2] = NULL;

    /* Set field options */
    set_field_back(form_input_fields[0], A_UNDERLINE);
    field_opts_off(form_input_fields[0], O_AUTOSKIP); /* Don't go to next field when this */
    /* Field is filled up         */
    set_field_back(form_input_fields[1], A_UNDERLINE); 
    field_opts_off(form_input_fields[1], O_AUTOSKIP);

    PANEL *panel_out; // Program output panel
    PANEL *panel_src; // Program source panel
    PANEL *panel_mem; // Program memory panel
    PANEL *panel_form;
    
    // Setup nCurses
	initscr();
    curs_set(0);
    cbreak();
    noecho();

    if (has_colors() == FALSE) {
        fprintf(stderr, "Terminal doesn't support color\n");
        endwin();
        return NULL;
    }
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    // Window widths
    uint16_t w_src;
    if (COLS % 2) { // If the terminal width is odd, add 1 to src panel width
        w_src = COLS * 0.5 + 1;
    } else {
        w_src = COLS * 0.5;
    }

    uint16_t w_mem = COLS * 0.5 + 1;
    uint16_t w_out = COLS;

    // Window heights
    uint16_t h_src = LINES * 0.75;
    uint16_t h_mem = LINES * 0.75;
    uint16_t h_out = LINES * 0.25;
    
    int h_input, w_input;
    // Getting form dimensions
    form_input = new_form(form_input_fields);
    scale_form(form_input, &h_input, &w_input);

    // Window x positions
    uint16_t x_src = 0;
    uint16_t x_mem = w_src - 1;
    uint16_t x_out = 0;
    uint16_t x_input = (COLS / 2) - (w_input / 2);

    // Window y positions
    uint16_t y_src = 0;
    uint16_t y_mem = 0;
    uint16_t y_out = h_src - 1;
    uint16_t y_input = (LINES / 2) - (h_input / 2);

    // Setup inotify
    int inotify_fd, inotify_wd;
    char *inotify_filename = "prog.out";

    inotify_fd = inotify_init1(IN_NONBLOCK);
    
    // Adding inotify_filename to watch list
    inotify_wd = inotify_add_watch(inotify_fd, inotify_filename, IN_MODIFY);
    if (inotify_wd == -1) {
        fprintf(stderr, "Failed to Watch: %s\n", inotify_filename);
    } else {
        fprintf(stderr, "Watching: %s\n", inotify_filename);
    }

    // Initializing windows and panels
    window_out = newwin(h_out, w_out, y_out, x_out);
    window_src = newwin(h_src, w_src, y_src, x_src);
    window_mem = newwin(h_mem, w_mem, y_mem, x_mem);
    panel_src = new_panel(window_src);
    panel_mem = new_panel(window_mem);
    panel_out = new_panel(window_out);

    // creating window_form
    window_form = newwin(h_input + 2, w_input + 2, y_input, x_input);
    keypad(window_form, TRUE);
    panel_form = new_panel(window_form);

    // Setting window for the form
    set_form_win(form_input, window_form);
    set_form_sub(form_input, derwin(window_form, h_input, w_input, 1, 1));

    // Update memory panel
    while (running) {
        // Update output panel
        int err;
        int i = 0;
        int length = 0;
        char buffer[INOTIFY_BUF_LEN];
        char c;
        long last_read = 0;
        int cursY = 1;
        int cursX = 1;

        length = read(inotify_fd, buffer, INOTIFY_BUF_LEN);
        while (i < length) {
            struct inotify_event *event = (struct inotify_event*) &buffer[i];

            if (event->mask & IN_MODIFY) {
                fprintf(stderr, "File %s was modified!\n", inotify_filename);
                // Seeking to the end of what we've read in the file so far
                fseek(fd_output, last_read, SEEK_SET);

                // Loop that reads the new lines of the file character by character
                c = fgetc(fd_output);
                while (c != EOF) {
                    if (c == '\n') {
                        cursY++;
                        cursX = 1;
                    }
                    else {
                        // Print the read character to the nCurses window
                        mvwaddch(window_out, cursY, cursX++, c);
                    }
                    // Read the next character
                    c = fgetc(fd_output);
                }
            }
            i += INOTIFY_EVENT_SIZE + event->len;
            last_read = ftell(fd_output);
        }

        // If source has yet to be printed
        if (!src_printed) {
            // If func_line_start was set by bad function call
            if (func_line_start) {
                print_current_function(window_src, filename);
                src_printed = 1;
            }
        }

        // Checking for user input signal
        pthread_mutex_lock(&mutex_buffer);
        if (input_requested) {
            fprintf(stderr, "T1: Input was requested\n");
            strncpy(buffer_input, "This is a test of the input system", 1024);
            sleep(6);
            // Signal main that input has been received
            fprintf(stderr, "T1: Signalling the main thread...\n");
            input_received = 1;
            input_requested = 0;
            fprintf(stderr, "T1: buffer contents \"%s\"\n", buffer);
            pthread_cond_signal(&cond_buffer);
        }
        pthread_mutex_unlock(&mutex_buffer);
        
        // Update memory panel
        print_stack(window_mem);

        box(window_out, 0, 0);
        box(window_src, 0, 0);
        box(window_mem, 0, 0);
        box(window_form, 0, 0);
        post_form(form_input);

        update_panels();
        doupdate();

        curs_set(1);
        form_driver(form_input, REQ_NEXT_FIELD);
        form_driver(form_input, REQ_PREV_FIELD);
        int ch;
        while((ch = wgetch(window_form)) != KEY_F(1))
        {
            switch(ch)
            {   
                default:
                /* If this is a normal character, it gets */
                /* Printed                */    
                form_driver(form_input, ch);
                break;
            }

            update_panels();
            doupdate();
        }
        curs_set(0);

        usleep(250000);
    }

    getch();
    // Cleanup
    del_panel(panel_out);
    del_panel(panel_src);
    del_panel(panel_mem);
    del_panel(panel_form);
    delwin(window_out);
    delwin(window_src);
    delwin(window_mem);
    delwin(window_form);
    endwin();
    return NULL;
}
