#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <png.h>
#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TILES_FILEPATH "assets/sprites/fantasy_tiles.png"
#define WALKING_FILEPATH "assets/sprites/walking-12px.png"

constexpr int TILE_SIZE = 64;
constexpr int PLAYER_SIZE = 48;

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

template <typename T>
T max(T n1, T n2) {
  return n1 > n2 ? n1 : n2;
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
    {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Empty, Tile::Empty},
};

struct Tile_Texture {
  SDL_Rect rect;
  SDL_Texture *texture;
};

void render_tile_texture(SDL_Renderer *renderer,
                         Tile_Texture texture,
                         SDL_Rect dstrect)
{
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
        SDL_Rect dstrect = {
            x * TILE_SIZE,
            y * TILE_SIZE,
            TILE_SIZE, TILE_SIZE};
        render_tile_texture(renderer,
                            wall_texture,
                            dstrect);
      }
      break;

      default:
        break;
      }
    }
  }
}

SDL_Texture *load_texture_from_png(SDL_Renderer *renderer, const char *filepath)
{
  // * Read Image using libpng
  png_image image; /* The control structure used by libpng */
  /* Initialize the 'png_image' structure. */
  memset(&image, 0, (sizeof image));
  image.version = PNG_IMAGE_VERSION;
  if(!png_image_begin_read_from_file(&image, filepath)) {
    png_image_free(&image);
    fprintf(stderr, "ERROR: libpng: could not read file: `%s`: %s\n", filepath, image.message);
    abort();
  }
  image.format = PNG_FORMAT_RGBA;

  /* 
  * Now allocate enough memory to hold the image in this format; the
  * PNG_IMAGE_SIZE macro uses the information about the image (width,
  * height and format) stored in 'image'.
  */
  png_bytep image_pixels;
  image_pixels = (png_bytep)malloc(PNG_IMAGE_SIZE(image));

  if (!png_image_finish_read(
          &image,
          nullptr /*background*/,
          image_pixels,
          0 /*row_stride*/,
          nullptr /*colormap*/))
  {
    fprintf(stderr, "ERROR: libpng: could not finish reading file: `%s`: %s\n", filepath, image.message);
    abort(); 
  }

  // * This is a sdl surface from png image
  SDL_Surface *image_surface =
      sdl(SDL_CreateRGBSurfaceFrom(image_pixels,
                                    image.width,
                                    image.height,
                                    32,
                                    image.width * 4,
                                    0x000000FF,
                                    0x0000FF00,
                                    0x00FF0000,
                                    0xFF000000));

  // * This is a sdl texture from sdl surface
  SDL_Texture *image_texture =
      sdl(SDL_CreateTextureFromSurface(renderer, image_surface));

  free(image_pixels);
  SDL_FreeSurface(image_surface);
  return image_texture;
}

int main(void) {
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

  SDL_Texture *tileset_texture =
      load_texture_from_png(renderer, TILES_FILEPATH);
  Tile_Texture tile_texture = {
    .rect = {120, 128, 16, 16},
    .texture = tileset_texture};

  SDL_Texture *walking_texture =
      load_texture_from_png(renderer, WALKING_FILEPATH);
  constexpr int walking_frame_count = 4;
  constexpr Uint64 walking_frame_duration = 100;
  int walking_frame_current = 0;
  Tile_Texture walking_frames[walking_frame_count];
  for (int i = 0; i < walking_frame_count; ++i) {
    walking_frames[i].rect = {
        .x = i * PLAYER_SIZE,
        .y = 0,
        .w = PLAYER_SIZE,
        .h = PLAYER_SIZE};
    walking_frames[i].texture = walking_texture;
  }

  Uint64 walking_frame_cooldown = walking_frame_duration;
  bool quit = false;
  int x = 0;

  const Uint8* keyboard = SDL_GetKeyboardState(NULL);
  while (!quit) {
    const Uint64 begin = SDL_GetTicks64();
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

    if(keyboard[SDL_SCANCODE_D]) {
      x += 1;
    }
    else if (keyboard[SDL_SCANCODE_A]) {
      x -= 1;
    }

    // * Update state
    // * Render state

    sdl(SDL_SetRenderDrawColor(renderer,
                               0x00, 0x00, 0x00, 0xff));

    sdl(SDL_RenderClear(renderer));
   
    render_level(renderer, tile_texture);
    SDL_Rect dstrect = {
        x,
        4 * TILE_SIZE - PLAYER_SIZE,
        PLAYER_SIZE, PLAYER_SIZE};
    render_tile_texture(renderer, walking_frames[walking_frame_current], dstrect);
    SDL_RenderPresent(renderer);

    const Uint64 dt = SDL_GetTicks64() - begin;
    if(dt < walking_frame_cooldown) {
      walking_frame_cooldown -= dt;
    }
    else {
      walking_frame_current = (walking_frame_current + 1) % walking_frame_count;
      walking_frame_cooldown = walking_frame_duration;
    }

  }

  SDL_Quit();
  return 0;
}