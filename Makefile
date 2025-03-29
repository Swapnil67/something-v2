PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra -Wconversion -pedantic -ggdb -std=c++17 `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)` -lm

game: main.cpp
	$(CXX) $(CXXFLAGS) -o game main.cpp $(LIBS)