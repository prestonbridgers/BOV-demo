CC=gcc
CFLAGS=-g -Wall -z execstack -fno-stack-protector -fno-pie
LFLAGS=-pthread -lpanel -lcurses -lform -z execstack -fno-stack-protector -fno-pie

all: demo1 demo2 demo3 launcher

launcher: launcher_src/launcher.c
	make -C launcher_src

demo1: bov.o ui.o demo1.o
	$(CC) $(LFLAGS) $^ -o $@

demo2: bov.o ui.o demo2.o
	$(CC) $(LFLAGS) $^ -o $@

demo3: bov.o ui.o demo3.o
	$(CC) $(LFLAGS) $^ -o $@

bov.o: bov.c 
	$(CC) $(CFLAGS) -c $< -o $@

ui.o: ui.c 
	$(CC) $(CFLAGS) -c $< -o $@

demo1.o: demo1.c
	$(CC) $(CFLAGS) -c $< -o $@

demo2.o: demo2.c
	$(CC) $(CFLAGS) -c $< -o $@

demo3.o: demo3.c
	$(CC) $(CFLAGS) -c $< -o $@

run:
	@./launcher 2> err.log

clean:
	rm demo1
	rm demo2
	rm demo3
	rm *.o
	make clean -C launcher_src
