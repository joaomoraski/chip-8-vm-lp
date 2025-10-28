# flags
CXXFLAGS = -std=c++17 -Wall -O2
INCLUDES = -Iinclude
SRC      = src/main.cpp src/chip8.cpp src/display.cpp src/keyboard.cpp
OBJ      = $(SRC:.cpp=.o)
BIN      = chip8

# validaçao se for ubuntu(riume) ou mac(moraski)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    # linux: libsdl2-dev instala em /usr/include/SDL2
    INCLUDES += -I/usr/include
    LIBS = -lSDL2
else ifeq ($(UNAME_S),Darwin)
    # macOs: libs em /opt/homebrew/Cellar/sdl2
    SDL2_PATH := /opt/homebrew/Cellar/sdl2/2.32.10
    INCLUDES += -I$(SDL2_PATH)/include
    LIBS = -L$(SDL2_PATH)/lib -lSDL2
endif

all: $(BIN)

$(BIN): $(OBJ)
	g++ $(OBJ) -o $(BIN) $(LIBS)

%.o: %.cpp
	g++ $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	./$(BIN) roms/IBM\ Logo.ch8 --scale 10 --clock 400

run-tank: $(BIN)
	./$(BIN) roms/TANK --scale 15 --clock 800


run-red: $(BIN)
	./$(BIN) roms/IBM\ Logo.ch8 --scale 10 --clock 400 --color 255 0 0

run-tank-red: $(BIN)
	./$(BIN) roms/TANK --scale 15 --clock 800 --color 255 0 0
