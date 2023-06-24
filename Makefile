PROJECT_NAME = pixel-viewer
RAYLIB_PATH = ../raylib/src

.PHONY: all rel run clean res icon

all:
    ifeq ($(OS),Windows_NT)
		gcc -o $(PROJECT_NAME) main.c $(PROJECT_NAME).res -s -O2 -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99 -Wall
    else
		gcc -o $(PROJECT_NAME) main.c -s -O2 -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif

rel:
	gcc -o $(PROJECT_NAME) main.c $(PROJECT_NAME).res -s -O2 -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99 -Wall -mwindows
	
run: all
	./$(PROJECT_NAME)

clean:
	rm $(PROJECT_NAME)

res:
	windres $(PROJECT_NAME).rc -O coff -o $(PROJECT_NAME).res

icon:
	magick convert icon.png -define icon:auto-resize=256,128,64,48,32,16 icon.ico