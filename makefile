CC=gcc
CFLAGS=-g -Wall -z execstack -fno-stack-protector
LFLAGS=-pthread -lpanel -lcurses -lform

bov: main.o ui.o
	$(CC) $(LFLAGS) $^ -o $@

main.o: main.c 
	$(CC) $(CFLAGS) -c $< -o $@

ui.o: ui.c 
	$(CC) $(CFLAGS) -c $< -o $@

run:
	@./bov 2> err.log

clean:
	rm bov
	rm *.o
