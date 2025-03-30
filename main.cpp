#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <png.h>
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
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
};

struct Tile_Texture {
  SDL_Rect rect;
  SDL_Texture *texture;
};

void render_tile_texture(SDL_Renderer *renderer,
                         Tile_Texture texture,
                         int x, int y)
{
  SDL_Rect dstrect = {x, y, TILE_SIZE, TILE_SIZE};
  sdl(SDL_RenderCopy(renderer,
                     texture.texture,
                     &texture.rect, // * srcrect 
                     &dstrect));
}

void render_level(SDL_Renderer *renderer, Tile_Texture wall_texture)
{
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch (level[y][x])
      {
      case Tile::Empty:
        /* code */
        break;
      case Tile::Wall:
      {
        // sdl(SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255));
        // SDL_Rect rect = {
        //     x * TILE_SIZE,
        //     y * TILE_SIZE,
        //     TILE_SIZE, TILE_SIZE};
        // sdl(SDL_RenderFillRect(renderer, &rect));

        render_tile_texture(renderer,
                            wall_texture,
                            x * TILE_SIZE,
                            y * TILE_SIZE);
      }
      break;

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

  // * Read Image using libpng
  const char *TILESET_FILEPATH = "assets/sprites/fantasy_tiles.png";
  png_image tilesset; /* The control structure used by libpng */
  /* Initialize the 'png_image' structure. */
  memset(&tilesset, 0, (sizeof tilesset));
  tilesset.version = PNG_IMAGE_VERSION;
  if(!png_image_begin_read_from_file(&tilesset, TILESET_FILEPATH)) {
    fprintf(stderr, "ERROR: libpng: could not read file: `%s`: %s\n", TILESET_FILEPATH, tilesset.message);
    abort();
  }
  // uint32_t *tilesset_buffer = (uint32_t *)std::malloc(sizeof(uint32_t) * tilesset.width * tilesset.height);

  tilesset.format = PNG_FORMAT_RGBA;
  /* 
  * Now allocate enough memory to hold the image in this format; the
  * PNG_IMAGE_SIZE macro uses the information about the image (width,
  * height and format) stored in 'image'.
  */
  png_bytep tileset_pixels;
  tileset_pixels = (png_bytep)malloc(PNG_IMAGE_SIZE(tilesset));

  if (!png_image_finish_read(
          &tilesset,
          nullptr /*background*/,
          tileset_pixels,
          0 /*row_stride*/,
          nullptr /*colormap*/))
  {
    fprintf(stderr, "ERROR: libpng: could not finish reading file: `%s`: %s\n", TILESET_FILEPATH, tilesset.message);
    abort(); 
  }

  // * This is a texture from png image
  SDL_Surface *tileset_surface =
      sdl(SDL_CreateRGBSurfaceFrom(tileset_pixels,
                                    tilesset.width,
                                    tilesset.height,
                                    32,
                                    tilesset.width * 4,
                                    0x000000FF,
                                    0x0000FF00,
                                    0x00FF0000,
                                    0xFF000000));

  SDL_Texture *tileset_texture =
      sdl(SDL_CreateTextureFromSurface(renderer, tileset_surface));

  Tile_Texture wall_texture = {
      .rect = {120, 128, 16, 16},
      .texture = tileset_texture};

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

   
    // sdl(SDL_RenderCopy(renderer,
    //                 tileset_texture,
    //                 &tileset_surface->clip_rect,
    //                 &tileset_surface->clip_rect));

    render_level(renderer, wall_texture);

    SDL_RenderPresent(renderer);
  }

  SDL_Quit();
  return 0;
}