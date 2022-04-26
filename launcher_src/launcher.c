#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <curses.h>
#include <menu.h>

/* GLOBALS */
char **choices;
char **descs;
int num_demos;
char *filename = "demos.txt";

void
read_demo_meta()
{
   char *line = NULL;
   size_t n = 0;
   int i;
   int num_lines = 0;
   FILE *fd = fopen(filename, "r");

   // Error checking
   if (fd == NULL) {
      perror(strerror(errno));
      exit(1);
   }

   // Getting the total number of lines;
   while(getline(&line, &n, fd) != -1) {
      num_lines++;
      free(line);
      line = NULL;
   }

   // Mallocing the choices and descs lists
   choices = calloc(num_lines, sizeof *choices);
   descs = calloc(num_lines, sizeof *choices);
   for (i = 0; i < num_lines; i++) {
      choices[i] = NULL;
      descs[i] = NULL;
   }

   // Moving back to the beginning of the file
   rewind(fd);

   // Filling choices and descs lists
   i = 0;
   while(getline(&line, &n, fd) != -1) {
      // Getting the demo path
      char *tok = strtok(line, "|");
      fprintf(stderr, "token 1: %s\n", tok);
      choices[i] = strdup(tok);

      // Getting the demo description
      tok = strtok(NULL, "|");
      fprintf(stderr, "token 2: %s\n", tok);
      descs[i] = strdup(tok);

      // Trimming the trailing \n char off of the description
      descs[i][strlen(descs[i]) - 1] = '\0';

      free(line);
      line = NULL;
      i++;
   }

   // Setting the global num_demos
   num_demos = num_lines;

   free(line);
   fclose(fd);
}

int
get_largest_choice_width() {
   int i;
   char line[COLS];
   int largest = 0;

   for (i = 0; i < num_demos; i++) {
      strncpy(line, choices[i], COLS);
      strcat(line, " | ");
      strcat(line, descs[i]);
      fprintf(stderr, "Length of line %d: %ld\n", i, strlen(line));

      if (strlen(line) > largest) {
         largest = strlen(line);
      }
   }

   return largest;
}

int
main(int argc, char **argv)
{
   read_demo_meta();

   ITEM **my_items;
   char *chosen_item;
   int c;
   MENU *my_menu;
   int i;
   WINDOW *win;
   int x_win, y_win;
   int h_win, w_win;
   char *win_title = "BOV Launcher: Please select a demo";

   // nCurses initialization
   initscr();
   curs_set(0);
   cbreak();
   noecho();
   keypad(stdscr, TRUE);

   //FILE *fd = fopen("tmp.log", "w");
   //fprintf(fd, "Col: %d\nLines: %d\n\n", COLS, LINES);
   //fclose(fd);
   // 130 x 43
   if (COLS < 130 || LINES < 43) {
      endwin();
      printf("Error: Terminal size too small.\n\tPlease resize your terminal to at least 130x43 for best results\n");
      return EXIT_SUCCESS;
   }

   w_win = get_largest_choice_width() + 10;
   h_win = num_demos + 2 + 6;
   x_win = COLS / 2 - (w_win / 2);
   y_win = LINES / 2 - h_win / 2;
   win = newwin(h_win, w_win, y_win, x_win);

   my_items = calloc(num_demos + 1, sizeof *my_items);
   for (i = 0; i < num_demos; i++) {
      my_items[i] = new_item(choices[i], descs[i]);
      set_item_userptr(my_items[i], choices[i]);
   }
   my_items[num_demos] = NULL;

   my_menu = new_menu(my_items);
   set_menu_win(my_menu, win);
   set_menu_sub(my_menu, derwin(win, num_demos, get_largest_choice_width(), h_win / 2 - num_demos / 2, w_win / 2 - get_largest_choice_width() / 2));

   mvprintw(LINES - 2, 4, "Press 'q' to Exit");
   post_menu(my_menu);

   wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
   mvwaddstr(win, 0, (w_win / 2) -  (strlen(win_title) / 2), win_title);

   wrefresh(stdscr);
   wrefresh(win);

   while ((c = getch()) != 'q') {
      switch (c) {
         case KEY_DOWN:
            menu_driver(my_menu, REQ_DOWN_ITEM);
            break;
         case KEY_UP:
            menu_driver(my_menu, REQ_UP_ITEM);
            break;
         case KEY_ENTER:
         case '\n':
         case '\r':
         {
            ITEM *cur;
            cur = current_item(my_menu);
            chosen_item = (char*) item_userptr(cur);
            fprintf(stderr, "User selected %s\n", chosen_item);

            char **env = calloc(2, sizeof *env);
            env[0] = calloc(256, sizeof(char));
            strncpy(env[0], chosen_item, 256);
            env[1] = NULL;
            endwin();
            execvp(chosen_item, env);
            perror(strerror(errno));
         }
            break;
      }
      wrefresh(win);
   }

   // nCurses cleanup
   for (i = 0; i < num_demos; i++) {
      free_item(my_items[i]);
   }
   free(my_items);
   free_menu(my_menu);
   delwin(win);
   endwin();
   return EXIT_SUCCESS;
}
