PROJECT_NAME = pixel-viewer
RAYLIB_PATH = ../raylib/src
CFLAGS = -s -std=gnu99 -Wall -Wextra

all:
    ifeq ($(OS),Windows_NT)
		make res
		gcc -o $(PROJECT_NAME) main.c $(CFLAGS) -g -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lopengl32 -lgdi32 -lwinmm $(PROJECT_NAME).res
    else
		gcc -o $(PROJECT_NAME) main.c $(CFLAGS) -g -fsanitize=address,undefined -fno-omit-frame-pointer -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif

rel:
    ifeq ($(OS),Windows_NT)
		make res
		gcc -o $(PROJECT_NAME) main.c $(CFLAGS) -O3 -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows $(PROJECT_NAME).res
    else
		gcc -o $(PROJECT_NAME) main.c $(CFLAGS) -O3 -I$(RAYLIB_PATH) -L$(RAYLIB_PATH) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
	./$(PROJECT_NAME)
	
run: all
	./$(PROJECT_NAME)

clean:
	rm $(PROJECT_NAME)

res:
	windres $(PROJECT_NAME).rc -O coff -o $(PROJECT_NAME).res

icon:
	magick convert icon.png -define icon:auto-resize=256,128,64,48,32,16 icon.ico
