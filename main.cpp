#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

constexpr int TILE_SIZE = 64;

int sdl(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError()); 
    abort();
  }
  return code;
}

template <typename T>
T *sdl(T *ptr) {
  if(ptr == nullptr) {
    fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());  
    abort();
  }
  return ptr;
}

enum class Tile
{
  Empty = 0,
  Wall
};

constexpr int LEVEL_WIDTH = 5;
constexpr int LEVEL_HEIGHT = 5;
Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
};

void render_level(SDL_Renderer *renderer)
{
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch (level[y][x])
      {
      case Tile::Empty:
        /* code */
        break;
      case Tile::Wall: {
        sdl(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
        SDL_Rect rect = {
            x * TILE_SIZE,
            y * TILE_SIZE,
            TILE_SIZE, TILE_SIZE};
        sdl(SDL_RenderFillRect(renderer, &rect));
      } break;
      
      default:
        break;
      }
    }
  }
}


template <typename T>
T max(T n1, T n2) {
  return n1 > n2 ? n1 : n2;
}

int main(void) {
  printf("My Game\n");

  sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

  SDL_Window *window = sdl(SDL_CreateWindow(
      "Game",
      0, 0,
      SCREEN_WIDTH, SCREEN_HEIGHT,
      SDL_WINDOW_RESIZABLE));

  SDL_Renderer *renderer = sdl(SDL_CreateRenderer(
      window,
      -1,
      SDL_RENDERER_PRESENTVSYNC));

  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        } break;
        case SDL_MOUSEWHEEL: {
        } break;
      }
    }

    // * Update state
    // * Render state

    sdl(SDL_SetRenderDrawColor(renderer,
                               0x00, 0x00, 0x00, 0xff));

    sdl(SDL_RenderClear(renderer));

    // int window_width, window_height;
    // SDL_GetWindowSize(window, &window_width,
    //                   &window_height);

    // int rows = window_height / TILE_SIZE;
    // int columns = window_width / TILE_SIZE;
    // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    // for(int row = 0; row < rows; ++row) {
    //   sdl(SDL_RenderDrawLine(renderer,
    //                          0,
    //                          (row + 1) * TILE_SIZE,
    //                          window_width,
    //                          (row + 1) * TILE_SIZE));
    // }
    // for(int col = 0; col < columns; ++col) {
    //   sdl(SDL_RenderDrawLine(renderer,
    //                          (col + 1) * TILE_SIZE,
    //                          0,
    //                          (col + 1) * TILE_SIZE,
    //                          window_height));
    // }

    render_level(renderer);

    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
  return 0;
}