CC = gcc
CFLAG = -Wall
LDFLAG = `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image
BUILD_DIR = build
OBJECT_DIR = obj


all: asciiArt Cshell nalloc

asciiArt: $(OBJECT_DIR)/asciiArt.o
	$(CC) $^ -o $(BUILD_DIR)/$@ $(LDFLAG)

$(OBJECT_DIR)/asciiArt.o: asciiArt.c
	$(CC) $(CFLAG) -o $@ -c $<

Cshell: $(OBJECT_DIR)/Cshell.o
	$(CC) $^ -o $(BUILD_DIR)/$@

$(OBJECT_DIR)/Cshell.o: Cshell.c
	$(CC) $(CFLAG) -o $@ -c $<

nalloc: $(OBJECT_DIR)/nalloc.o
	$(CC) $^ -o $(BUILD_DIR)/$@

$(OBJECT_DIR)/nalloc.o: nalloc.c
	$(CC) $(CFLAG) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)/
	rm -rf $(OBJECT_DIR)/
	mkdir -p $(BUILD_DIR)/
	mkdir -p $(OBJECT_DIR)/