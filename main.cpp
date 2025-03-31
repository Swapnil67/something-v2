#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <png.h>
#include <cassert>
#include <SDL.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TILES_FILEPATH "assets/sprites/fantasy_tiles.png"
#define WALKING_FILEPATH "assets/sprites/walking-12px.png"

#define COLOR_BLACK 0x00, 0x00, 0x00, 0xff
#define COLOR_RED 0xff, 0x00, 0x00, 0xff

constexpr int TILE_SIZE = 64;
constexpr int PLAYER_SIZE = 48;
constexpr int PLAYER_SPEED = 2;

int sec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError()); 
    abort();
  }
  return code;
}

template <typename T>
T *sec(T *ptr) {
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
    {Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty},
    {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall},
};

struct Sprite {
  SDL_Rect rect;
  SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   SDL_Rect dstrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  sec(SDL_RenderCopyEx(renderer,
                       texture.texture,
                       &texture.rect, // * srcrect
                       &dstrect,
                       0.0, nullptr,
                       flip));
}

void render_level(SDL_Renderer *renderer, Sprite wall_texture)
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
            render_sprite(renderer,
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
      sec(SDL_CreateRGBSurfaceFrom(image_pixels,
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
      sec(SDL_CreateTextureFromSurface(renderer, image_surface));

  free(image_pixels);
  SDL_FreeSurface(image_surface);
  return image_texture;
}

struct Animat {
  Sprite *frames;
  size_t frames_count;
  size_t frame_current;
  uint64_t frame_duration;
  uint64_t frame_cooldown;
};

static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   SDL_Rect dstrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  render_sprite(renderer,
                animat.frames[animat.frame_current % animat.frames_count],
                dstrect, flip);
}

// * Checks the animation cooldown period before rendering new animation
void update_animat(Animat *animat, Uint64 dt) {
  if (dt < animat->frame_cooldown) {
    animat->frame_cooldown -= dt;
  }
  else {
    animat->frame_current = (animat->frame_current + 1) % animat->frames_count;
    animat->frame_cooldown = animat->frame_duration;
  }
}

struct Player {
  int dy;
  SDL_Rect hitbox;
};

static inline
bool is_not_oob(int x, int y) {
  return 0 <= x && x < LEVEL_WIDTH &&
         0 <= y && y < LEVEL_HEIGHT;
}

void resolve_player_collision(Player *player) {
  assert(player);

  // * bottom left hitbox point
  int x0 = player->hitbox.x / TILE_SIZE;
  
  // * bottom right hitbox point
  int x1 = (player->hitbox.x + TILE_SIZE) / TILE_SIZE;
  
  // * top left hitbox point
  int y0 = player->hitbox.y / TILE_SIZE;

  // * top right hitbox point
  int y1 = (player->hitbox.y + TILE_SIZE) / TILE_SIZE;

  // printf("x0 = %d, x1 = %d, y = %d\n", x0, x1,  y);
  for (int x = x0; x <= x1; ++x) {
    if (is_not_oob(x, y0) && level[y0][x] == Tile::Wall) {
      player->dy = 0;
      // * Snap the player back to bottom
      player->hitbox.y = (y0 + 1) * TILE_SIZE; // * Co-ordinates of tile just below the wall
    }

    if (is_not_oob(x, y1) && level[y1][x] == Tile::Wall) {
      player->dy = 0;
      // * Put player on top of the wall
      player->hitbox.y = y1 * TILE_SIZE - player->hitbox.h;
      return;
    }
  }
}

int main(void) {
  sec(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window = sec(SDL_CreateWindow(
      "Game",
      0, 0,
      SCREEN_WIDTH, SCREEN_HEIGHT,
      SDL_WINDOW_RESIZABLE));

  SDL_Renderer *renderer = sec(SDL_CreateRenderer(
      window, -1,
      SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // * Tile Texture
  SDL_Texture *tileset_texture =
      load_texture_from_png(renderer, TILES_FILEPATH);
  Sprite tile_texture = {
    .rect = {120, 128, 16, 16},
    .texture = tileset_texture};

  // * Player Texture
  SDL_Texture *walking_texture =
      load_texture_from_png(renderer, WALKING_FILEPATH);
  constexpr int walking_frame_count = 4;
  Sprite walking_frames[walking_frame_count];
  for (int i = 0; i < walking_frame_count; ++i) {
    walking_frames[i].rect = {
        .x = i * PLAYER_SIZE,
        .y = 0,
        .w = PLAYER_SIZE,
        .h = PLAYER_SIZE};
    walking_frames[i].texture = walking_texture;
  }

  // * Player Animation
  Animat walking = {
      .frames = walking_frames,
      .frames_count = 4,
      .frame_current = 0,
      .frame_duration = 100,
      .frame_cooldown = 100};

  // * Player Idle Animation
  Animat idle = {
      .frames = walking_frames + 2, // * 3rd frame
      .frames_count = 1,
      .frame_current = 0,
      .frame_duration = 100,
      .frame_cooldown = 0};

  // * Current player animation
  Animat *current = &idle;

  // * Define Player
  Player player = {
      .dy = 0,
      .hitbox = {0, 0, PLAYER_SIZE, PLAYER_SIZE}};
      
  SDL_RendererFlip player_dir = SDL_FLIP_NONE;
      
  int ddy = 1;
  bool quit = false, debug = false;
  const Uint8* keyboard = SDL_GetKeyboardState(NULL);

  while (!quit) {
    const Uint64 begin = SDL_GetTicks64();
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        } break;
        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
          case SDLK_SPACE: {
            player.dy = -20;
          } break;
          case SDLK_q: {
            debug = !debug;
          } break;
          case SDLK_r: {
            player.dy = 0;
            player.hitbox.x = 0;
            player.hitbox.y = 0;
          } break;

          default:
            break;
          }
        } break;
      }
    }

    player.dy += ddy;
    player.hitbox.y += player.dy;

    // * Resolve player collision
    resolve_player_collision(&player);

    // * Update state
    if(keyboard[SDL_SCANCODE_D]) {
      player.hitbox.x += PLAYER_SPEED;
      current = &walking;
      player_dir = SDL_FLIP_NONE;
    }
    else if (keyboard[SDL_SCANCODE_A]) {
      player.hitbox.x -= PLAYER_SPEED;
      current = &walking;
      player_dir = SDL_FLIP_HORIZONTAL;
    }
    else {
      current = &idle;
      player_dir = SDL_FLIP_NONE;
    }

    // * Render state
    sec(SDL_SetRenderDrawColor(renderer, COLOR_BLACK));
    sec(SDL_RenderClear(renderer));
    render_level(renderer, tile_texture);
    render_animat(renderer, *current, player.hitbox, player_dir);

    // * Show player hitbox
    if(debug) {
      sec(SDL_SetRenderDrawColor(renderer, COLOR_RED));
      SDL_RenderDrawRect(renderer, &player.hitbox);
    }

    SDL_RenderPresent(renderer);
    update_animat(&walking, SDL_GetTicks64() - begin);
  }

  SDL_Quit();
  return 0;
}