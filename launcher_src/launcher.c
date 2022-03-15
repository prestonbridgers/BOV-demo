#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <curses.h>
#include <menu.h>

int
main(int argc, char **argv)
{
   // Variables
   char *choices[] = {
      "./demo1",
      "./demo2"
   };

   char *desc[] = {
      "| Simple integer overflow",
      "| Overwriding a return address"
   };

   ITEM **my_items;
   char *chosen_item;
   int c;
   MENU *my_menu;
   int n_choices, i;
   WINDOW *win;
   int x_win, y_win;
   int h_win, w_win;

   // nCurses initialization
   initscr();
   curs_set(0);
   cbreak();
   noecho();
   keypad(stdscr, TRUE);

   n_choices = (sizeof choices) / (sizeof choices[0]);

   x_win = COLS / 2 - 25;
   y_win = LINES / 2 - (sizeof choices) / 2;
   h_win = n_choices + 2;
   w_win = 50;
   win = newwin(h_win, w_win, y_win, x_win);

   my_items = calloc(n_choices + 1, sizeof *my_items);
   for (i = 0; i < n_choices; i++) {
      my_items[i] = new_item(choices[i], desc[i]);
      set_item_userptr(my_items[i], choices[i]);
   }
   my_items[n_choices] = NULL;

   my_menu = new_menu(my_items);
   set_menu_win(my_menu, win);
   set_menu_sub(my_menu, win);

   mvprintw(LINES - 2, 0, "Press 'q' to Exit");

   post_menu(my_menu);

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

            char **env = calloc(1, sizeof *env);
            env[0] = calloc(256, sizeof(char));
            strncpy(env[0], chosen_item, 256);
            env[1] = NULL;
            free_item(my_items[0]);
            free_item(my_items[1]);
            endwin();
            execvp(chosen_item, env);
            perror(strerror(errno));
         }
            break;
      }
      wrefresh(win);
   }

   


   // nCurses cleanup
   free_item(my_items[0]);
   free_item(my_items[1]);
   endwin();
   return EXIT_SUCCESS;
}
