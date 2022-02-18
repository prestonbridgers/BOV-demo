CC=gcc
CFLAGS=-g -Wall -z execstack -fno-stack-protector
LFLAGS=-pthread -lpanel -lcurses -lform

bov: bov.o ui.o demo1.o
	$(CC) $(LFLAGS) $^ -o $@

bov.o: bov.c 
	$(CC) $(CFLAGS) -c $< -o $@

ui.o: ui.c 
	$(CC) $(CFLAGS) -c $< -o $@

demo1.o: demo1.c
	$(CC) $(CFLAGS) -c $< -o $@

run:
	@./bov 2> err.log

clean:
	rm bov
	rm *.o
