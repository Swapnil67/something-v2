PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra -Wunused-function -Wconversion -pedantic -ggdb -std=c++20 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

game: src/scu.cpp src/error.cpp src/vec2.cpp src/sprite.cpp src/level.cpp src/projectile.cpp src/entity.cpp src/main.cpp
	g++ $(CXXFLAGS) -o game src/scu.cpp $(LIBS)