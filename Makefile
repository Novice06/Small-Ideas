CC = gcc
CFLAG = -Wall

#depends on how you've installed sdl
LDFLAG = `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image

TARGET = asciiart


all: bin/$(TARGET)

bin/$(TARGET): obj/main.o
	$(CC) $^ -o $@ $(LDFLAG)

obj/main.o: src/main.c
	$(CC) $(CFLAG) -o $@ -c $<

clean:
	rm -f obj\*.o

mproper: clean
	rm -f bin\$(TARGET)