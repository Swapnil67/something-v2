#include <cstdio>
#include <cstdlib>
#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

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
      }
    }

    // * Update state
    // * Render state

    sdl(SDL_SetRenderDrawColor(renderer,
                               0x00, 0x00, 0x00, 0xff));

    sdl(SDL_RenderClear(renderer));
    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
  return 0;
}