CC=gcc
CFLAGS=-Wall -g
LFLAGS=-lcurses -lmenu

default: launcher

launcher: launcher.c
	$(CC) $(CFLAGS) $(LFLAGS) $^ -o $@
	mv $@ ..

clean:
	rm ../launcher
