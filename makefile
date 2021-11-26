CC=gcc
CFLAGS=-g -Wall
LFLAGS=-pthread -lpanel -lcurses

bovis: main.o ui.o
	$(CC) $(LFLAGS) $^ -o $@

main.o: main.c 
	$(CC) $(CFLAGS) -c $< -o $@

ui.o: ui.c 
	$(CC) $(CFLAGS) -c $< -o $@

run:
	@./bovis 2> err.log

clean:
	rm bovis
	rm *.o
	rm *.log
	rm prog.out
