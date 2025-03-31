PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra -Wunused-function -Wconversion -pedantic -ggdb -std=c++20 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

game: main.cpp
	$(CXX) $(CXXFLAGS) -o game main.cpp $(LIBS)