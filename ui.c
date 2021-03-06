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
#include <ctype.h>
#include "bovis.h"
#include "bovis_globals.h"

/** Function gets the length of the next word.
 *  Assumes s points to a string beginning at the
 *  start of a word.
 *
 *  Ex: s -> "hello world!"  | good
 *      s -> " hello world!" | bad
 *
 *  s - The string of which to get the next word length.
 *
 *  returns - the length of the next word in s.
 */
int
bov_word_length(char *s)
{
    int wlen = 0;
    size_t i = 0;

    if (s == NULL) {
        return -1;
    }

    while (s[i] != ' ' && s[i] != '\0') {
        wlen++;
        i++;
    }

    return wlen;
}

/**
 * Prints to the program output panel.
 */
void
bov_print(char *s) {
    fprintf(fd_output, s);
    fflush(fd_output);
    return;
}

/**
 * This function creates a popup displaying the given string.
 * The user is instructed to press 'q' to close the popup.
 *
 * str - The string to be printed in the popup.
 */
void
bov_ui_popup(char *str)
{
    WINDOW *win;
    PANEL *pan;
    int width, height;
    int xpos, ypos;
    float ratio = 0.7;
    size_t i;
    size_t xcurs = 1;
    size_t ycurs = 1;

    width = COLS * ratio;
    height = LINES * ratio;

    xpos = COLS * (1 - ratio) / 2;
    ypos = LINES * (1 - ratio) / 2;

    win = newwin(height, width, ypos, xpos);
    pan = new_panel(win);
    top_panel(pan);

    wborder(win, '|','|','-','-','+','+','+','+');

    for (i = 0; i < strlen(str); i++) {
        // IF: between words
        if (str[i] == ' ') {
            // IF: next word is too long to fit in the popup
            int len = bov_word_length(&str[i+1]);
            if (xcurs + len >= width - 1) {
                // Make a new line
                ycurs++;
                xcurs = 1;
                // Skip printing the space by incrementing i again
                i++;
            }
        }

        // IF: a \n char is found, make a new line
        if (str[i] == '\n') {
            ycurs++;
            xcurs = 1;
            continue;
        }

        mvwaddch(win, ycurs, xcurs, str[i]);
        xcurs++;
    }

    update_panels();
    doupdate();
    
    int c;
    int popup_shown = 1;
    while ((c = getch()) == 's') {
      if (popup_shown) {   
         hide_panel(pan);
      } else {
         show_panel(pan);
      }
      popup_shown = !popup_shown;
      update_panels();
      doupdate();
    }
    

//    delwin(win);
//    del_panel(pan);
    return;
}

/* Prints the calling function to an nCurses window.
 *
 * Usage - This function must be the first statement of the calling function.
 *
 * window - The nCurses window to be printed
 */
void
print_current_function(WINDOW *win, char *filename)
{
    FILE *fd = fopen(filename, "r");
    int current_line = 0;
    char *line;
    int maxBuf = 256;
    short queue = 0;
    short queue_initialized = 0;
    int y_pos = 1;

    line = malloc(maxBuf);

    // Fast foward to the first line of the function
    while (current_line != func_line_start - 4) {
        getline(&line, (size_t*) &maxBuf, fd);
        current_line++;
    }

    // Loop prints each line of the function
    while (getline(&line, (size_t*) &maxBuf, fd) > 0)
    {
        // Ignore lines with a /* IGNORE */ tag in them
        if (strstr(line, "/* IGNORE */") != NULL)
            continue;

        // Queue management to tell when we're out of the function
        if (strstr(line, "{") != NULL) {
            queue++;
            queue_initialized = 1;
        }
        if (strstr(line, "}") != NULL) {
            queue--;
        }

        mvwaddstr(win, y_pos, 1, line);
        y_pos++;

        // Stop once we're out of the function
        if (queue == 0 && queue_initialized) {
            break;
        }
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
    /* fprintf(stderr, "%#010" PRIx32 "\n", high); */
    /* fprintf(stderr, "%#010" PRIx32 "\n", low); */
    /* fprintf(stderr, "%#018" PRIx64 "\n\n", *line_ptr); */

    char *line_ptr_v = (char*) line_ptr;
    char *buf_ptr_v  = (char*) buf_ptr;
    if (line_ptr_v == buf_ptr_v) {
        wattron(win, COLOR_PAIR(GREEN_PAIR));
        /* mvwprintw(win, ypos, 2, "%18p: %#018" PRIx64, *line_ptr, *line_ptr); */
        mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr);
        wprintw(win, " <- Buffer");
        wattroff(win, COLOR_PAIR(GREEN_PAIR));
    }
    //TODO: Calculate buf_ptr + 1 based on the size of the buffer that should
    //      be passed to this function as a paramater.
    /* else if (line_ptr_v == buf_ptr_v) { */
    /*     wattron(win, COLOR_PAIR(GREEN_PAIR)); */
    /*     mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr); */
    /*     wattroff(win, COLOR_PAIR(GREEN_PAIR)); */
    /* } */
    else if (line_ptr_v == (char*)ret_ptr) {
        wattron(win, COLOR_PAIR(RED_PAIR));
        /* mvwprintw(win, ypos, 2, "%#018" PRIx64, *line_ptr); */
        /* mvwprintw(win, ypos, 2, "%#018: " PRIx64 "%#018" PRIx64, line_ptr, *line_ptr); */
        mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr);
        wprintw(win, " <- Ret. Addr.");
        wattroff(win, COLOR_PAIR(RED_PAIR));
    }
    else if (line_ptr_v == (char*)int_ptr) {
        wattron(win, COLOR_PAIR(YELLOW_PAIR));
        /* mvwprintw(win, ypos, 2, "%#018" PRIx64, *line_ptr); */
        /* mvwprintw(win, ypos, 2, "%#018: " PRIx64 "%#018" PRIx64, line_ptr, *line_ptr); */
        mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr);
        wprintw(win, " <- int x");
        wattroff(win, COLOR_PAIR(YELLOW_PAIR));
    }
    // BETWEEN END OF BUFFER AND RET ADDR
    else if (line_ptr_v > buf_ptr_v && line_ptr_v < (char*) ret_ptr) {
        wattron(win, COLOR_PAIR(YELLOW_PAIR));
        /* mvwprintw(win, ypos, 2, "%#018" PRIx64, *line_ptr); */
        /* mvwprintw(win, ypos, 2, "%#018: " PRIx64 "%#018" PRIx64, line_ptr, *line_ptr); */
        mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr);
        wattroff(win, COLOR_PAIR(YELLOW_PAIR));
    }
    else if (line_ptr_v == (char*)stack_ptr) {
        /* mvwprintw(win, ypos, 2, "%#018" PRIx64, *line_ptr); */
        /* mvwprintw(win, ypos, 2, "%#018: " PRIx64 "%#018" PRIx64, line_ptr, *line_ptr); */
        mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr);
        wprintw(win, " <- %%RSP");
    }
    else {
        /* mvwprintw(win, ypos, 2, "%#018" PRIx64, *line_ptr); */
        mvwprintw(win, ypos, 2, "0x%016x: %#018" PRIx64, line_ptr, *line_ptr);
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
 * Prints a title int the middle of the top thing.
 * Note: call after boxing the window/drawing the window's border
 */
void
print_title(WINDOW *win, int width, char *title) {
    mvwaddstr(win, 0, (width / 2) - (strlen(title) / 2), title);
    mvwchgat(win, 0, width / 2 - strlen(title) / 2, strlen(title),
            A_BOLD | A_UNDERLINE, WINDOW_TITLE_COLOR, NULL);
    return;
}


/**
 * The nCurses thread run function.
 */
void*
cthread_run(void *arg)
{
    // Variables
    short src_printed = 0;

    WINDOW *window_out; // Program output window
    WINDOW *window_src; // Program source window
    WINDOW *window_mem; // Program memory window

    PANEL *panel_out; // Program output panel
    PANEL *panel_src; // Program source panel
    PANEL *panel_mem; // Program memory panel
    
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
    init_pair(4, COLOR_CYAN, COLOR_BLACK);

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

    // Window x positions
    uint16_t x_src = 0;
    uint16_t x_mem = w_src - 1;
    uint16_t x_out = 0;

    // Window y positions
    uint16_t y_src = 0;
    uint16_t y_mem = 0;
    uint16_t y_out = h_src;

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

    // Update memory panel
    while (running) {
        // Update output panel
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
            get_user_input();

            input_received = 1;
            input_requested = 0;
            pthread_cond_signal(&cond_buffer);
        }
        pthread_mutex_unlock(&mutex_buffer);

        // Check for popup_needed
        pthread_mutex_lock(&mutex_popup);
        if (popup_requested) {
           fprintf(stderr, "FROM UI: %s\n", popup_string);
           bov_ui_popup(popup_string);

           // Bring the old windows to the front:
           top_panel(panel_out);
           top_panel(panel_src);
           top_panel(panel_mem);

           popup_done = 1;
           popup_requested = 0;
           pthread_cond_signal(&cond_popup);
        }
        pthread_mutex_unlock(&mutex_popup);
        
        // Update memory panel
        if (update_mem) {
            print_stack(window_mem);
        }

        wborder(window_out, '|','|','-','-','+','+','+','+');
        wborder(window_src, '|','|','-','-','+','+','+','+');
        wborder(window_mem, '|','|','-','-','+','+','+','+');
        print_title(window_out, w_out, "Program Output");
        print_title(window_src, w_src, "Vulnerable Code");
        print_title(window_mem, w_mem, "Stack Memory Dump");

        update_panels();
        doupdate();


        usleep(250000);
    }

    //getch();
    // Cleanup
    del_panel(panel_out);
    del_panel(panel_src);
    del_panel(panel_mem);
    delwin(window_out);
    delwin(window_src);
    delwin(window_mem);
    endwin();
    return NULL;
}

/*
 * Initiates the user input process. Creating the appropriate windows,
 * forms, fields, etc. Gets the user's input and stores it in buffer_input
 * global char buffer.
 */
void
get_user_input()
{
    WINDOW *win_form;
    PANEL *pan_form;
    FORM *form;
    FIELD *fields[2];
    int w_form, h_form, x_form, y_form;

    // Init fields
    fields[0] = new_field(1, 35, 0, 0, 0, 0);
    fields[1] = NULL;

    set_field_back(fields[0], A_UNDERLINE);
    field_opts_off(fields[0], O_AUTOSKIP);

    form = new_form(fields);

    // Getting form dimensions
    scale_form(form, &h_form, &w_form);
    x_form = (COLS / 2) - (w_form / 2);
    y_form = (LINES / 2) - (h_form / 2);

    // creating window_form
    win_form = newwin(h_form + 2, w_form + 2, y_form, x_form);
    keypad(win_form, TRUE);
    pan_form = new_panel(win_form);

    // Setting window for the form
    set_form_win(form, win_form);
    set_form_sub(form, derwin(win_form, h_form, w_form, 1, 1));

    // Draw form window and post form
    top_panel(pan_form);
    box(win_form, 0, 0);
    print_title(win_form, w_form, "Enter an input string");
    post_form(form);

    update_panels();
    doupdate();

    // Main input loop
    curs_set(1);
    form_driver(form, REQ_NEXT_FIELD);
    form_driver(form, REQ_PREV_FIELD);
    int ch;
    int done = 0;
    while(!done)
    {
        ch = wgetch(win_form);
        switch(ch)
        {   
            case KEY_BACKSPACE:
                form_driver(form, REQ_DEL_PREV);
                break;
            case 10: // Enter
                form_driver(form, REQ_NEXT_FIELD);
                form_driver(form, REQ_PREV_FIELD);
                form_driver(form, REQ_END_LINE);
                strncpy(buffer_input,
                        trim_whitespace(field_buffer(fields[0], 0)),
                        1024);
                done = 1;
                break;
            default:
                form_driver(form, ch);
                break;
        }

        update_panels();
        doupdate();
    }
    curs_set(0);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    delwin(win_form);
    del_panel(pan_form);
}

/* Trims the leading and trailing whitespace of a given string and returns 
 * the a pointer to it.
 *
 * str - The string to be modified.
 */
char* trim_whitespace(char *str)
{
    if (str == NULL)
    {
        /* fprintf(stderr, "trim_whitespace: NULL string\n"); */
        exit(1);
    }
    char *end;

    //trim leading space
    while(isspace(*str))
        str++;

    if(*str == 0) // all spaces?
        return str;

    // trim trailing space
    end = str + strnlen(str, 128) - 1;

    while(end > str && isspace(*end))
        end--;

    // write new null terminator
    *(end+1) = '\0';

    return str;
}
