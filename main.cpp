#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <png.h>
#include <cassert>
#include <SDL.h>
#include <SDL_ttf.h>

#include "vec2.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TILES_FILEPATH "assets/sprites/fantasy_tiles.png"
#define WALKING_FILEPATH "assets/sprites/walking-12px.png"
#define PROJECTILE_SPARK_FILEPATH "assets/sprites/spark-sheet.png"
#define PROJECTILE_DESTORY_FILEPATH "assets/sprites/destroy-sheet.png"

#define COLOR_BLACK 0x00, 0x00, 0x00, 0xff
#define COLOR_RED 0xff, 0x00, 0x00, 0xff
#define COLOR_YELLOW 0xff, 0xff, 0x00, 0xff

const int TILE_SIZE = 64;
const int PLAYER_SPEED = 2;
const int PLAYER_TEXBOX_SIZE = 48;
const int PLAYER_HITBOX_SIZE = (PLAYER_TEXBOX_SIZE - 10);

template <typename T>
T *stec(T *ptr) {
  if(ptr == nullptr) {
    fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError()); 
    abort();
  }
  return ptr;
}

void stec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError()); 
    abort();
  }
}

void sec(int code) {
  if(code < 0) {
    fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError()); 
    abort();
  }
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

const int LEVEL_WIDTH = 10;
const int LEVEL_HEIGHT = 10;
Tile level[LEVEL_HEIGHT][LEVEL_WIDTH] = {
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Wall,  Tile::Empty, Tile::Wall,  Tile::Empty, },
  {Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall,  Tile::Wall, },
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
  {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, }};

struct Sprite {
  SDL_Rect rect;
  SDL_Texture *texture;
};

static inline
bool is_tile_inbounds(Vec2i p) {
  return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2i p) {
  return !is_tile_inbounds(p) || level[p.y][p.x] == Tile::Empty;
}

// * Render sprite with sprite dest rect
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

// * Render sprite with sprite position vector
void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   Vec2i pos,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  SDL_Rect dstrect = {
      pos.x - (texture.rect.w / 2), pos.y - (texture.rect.h / 2),
      texture.rect.w, texture.rect.h};
  sec(SDL_RenderCopyEx(renderer,
                       texture.texture,
                       &texture.rect, // * srcrect
                       &dstrect,
                       0.0,
                       nullptr,
                       flip));
}

void render_level(SDL_Renderer *renderer, Sprite top_ground_texture, Sprite bottom_ground_texture)
{
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch (level[y][x])
      {
      case Tile::Empty:
        break;
      case Tile::Wall:
      {
        SDL_Rect dstrect = {
            x * TILE_SIZE,
            y * TILE_SIZE,
            TILE_SIZE, TILE_SIZE};
        if (is_tile_empty(vec2(x, y - 1)))
        {
          render_sprite(renderer,
                        top_ground_texture,
                        dstrect);
        }
        else
        {
          render_sprite(renderer,
                          bottom_ground_texture,
                          dstrect);
        }
      }
      break;

      default:
        break;
      }
    }
  }
}

// * Creates a SDL_Texture from the png image
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
          0       /*row_stride*/,
          nullptr /*colormap*/))
  {
    fprintf(stderr, "ERROR: libpng: could not finish reading file: `%s`: %s\n", filepath, image.message);
    abort(); 
  }

  // * This is a sdl surface from png image
  SDL_Surface *image_surface =
      sec(SDL_CreateRGBSurfaceFrom(image_pixels,
                                   (int)image.width,
                                   (int)image.height,
                                   32,
                                   (int)image.width * 4,
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

// * Animat position as SDL_Rect
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

// * Animat position as Vec2i
static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   Vec2i pos,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
  render_sprite(renderer,
                animat.frames[animat.frame_current % animat.frames_count],
                pos, flip);
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

enum class Entity_Dir {
  Right = 0,
  Left
};

struct Entity {
  Vec2i pos;
  Vec2i vel;
  SDL_Rect texbox;
  SDL_Rect hitbox;

  Animat idle;
  Animat walking;
  Animat *current;

  Entity_Dir dir;
};

static inline
int get_sqr_dist(Vec2i p0, Vec2i p1) {
  auto d = p0 - p1;
  return d.x * d.x + d.y * d.y;
}

void resolve_point_collision(Vec2i *p) {
  assert(p);

  const Vec2i tile = *p / TILE_SIZE;
  // printf("tile_x: %d, tile_y: %d\n", tile.x, tile.y);

  // * check if player out of bound or standing on empty tile
  if(is_tile_empty(tile)) {
    return;
  }

  const Vec2i p0 = tile * TILE_SIZE;
  const Vec2i p1 = (tile + 1) * TILE_SIZE;

  struct Side {
    int sqr_distance;
    // int x, y;
    Vec2i np; // * neighbor positions
    // int dx, dy,
    Vec2i nd; // * neighbor direction
    int dd;
  };

  const size_t TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

  Side sides[] = {
      {get_sqr_dist({p0.x, 0}, {p->x, 0}), {p0.x, p->y}, {-1, 0}, TILE_SIZE_SQR},            // * Left side
      {get_sqr_dist({p1.x, 0}, {p->x, 0}), {p1.x, p->y}, {1, 0}, TILE_SIZE_SQR},             // * Right side
      {get_sqr_dist({0, p0.y}, {0, p->y}), {p->x, p0.y}, {0, -1}, TILE_SIZE_SQR},            // * Top side
      {get_sqr_dist({0, p1.y}, {0, p->y}), {p->x, p1.y}, {0, 1}, TILE_SIZE_SQR},             // * Bottom side
      {get_sqr_dist({p0.x, p0.y}, {p->x, p->y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // * Top left
      {get_sqr_dist({p1.x, p0.y}, {p->x, p->y}), {p1.x, p0.y}, {1, -1}, TILE_SIZE_SQR * 2},  // * Top right
      {get_sqr_dist({p0.x, p1.y}, {p->x, p->y}), {p0.x, p1.y}, {-1, 1}, TILE_SIZE_SQR * 2},  // * Bottom left
      {get_sqr_dist({p1.x, p1.y}, {p->x, p->y}), {p1.x, p1.y}, {1, 1}, TILE_SIZE_SQR * 2},   // * Bottom right
  };

  const int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

  // printf("*x - x0: %d, \t| *x: %d, x0: %d\n", std::abs(*x - x0), *x, x0);
  // printf("x1 - *x: %d, \t| x1: %d, *x: %d\n", std::abs(x1 - *x), x1, *x);
  // printf("*y - y0: %d, \t| *y: %d, y0: %d\n", std::abs(*y - y0), *y, y0);
  // printf("y1 - *y: %d, \t| y1: %d, *y: %d\n", std::abs(y1 - *y), y1, *y);
  // printf("-----------------\n");
  // printf("y0: %d, y1: %d\n", y0, y1);

  int closest_side = -1;
  // * Find to which current_side the distance is closest
  for (int current_side = 0; current_side < SIDES_COUNT; current_side++) {

    // * Check for neighbouring tiles
    // * If neighbouring tile is wall, increase the sqr_distance by TILE_SIZE
    for (int i = 1; !is_tile_empty(tile + sides[current_side].nd * i); ++i)
    {
      sides[current_side].sqr_distance += sides[current_side].dd;
    }

    if (closest_side < 0 || sides[closest_side].sqr_distance > sides[current_side].sqr_distance) {
      closest_side = current_side;
    }
  }

  *p = sides[closest_side].np;
}

void resolve_entity_collision(Entity *entity) {
  assert(entity);

  Vec2i p0 = vec2(entity->hitbox.x, entity->hitbox.y) + entity->pos;
  Vec2i p1 = p0 + vec2(entity->hitbox.w, entity->hitbox.h);

  Vec2i mesh[] = {
     p0,
     {p1.x, p0.y},
     {p0.x, p1.y},
     p1,
  };

  const int IMPACT_THRESHOLD = 3;
  const int MESH_COUNT = sizeof(mesh) / sizeof(mesh[0]);
  for (int i = 0; i < MESH_COUNT; ++i) {
    Vec2i t = mesh[i];

    resolve_point_collision(&t);
    Vec2i d = t - mesh[i]; 

    // printf("dx: %d, dy: %d\n", d.x, d.y);
    if (std::abs(d.y) >= IMPACT_THRESHOLD) {
      entity->vel.y = 0;
    }

    if (std::abs(d.x) >= IMPACT_THRESHOLD) {
      entity->vel.x = 0;
    }

    for(int j = 0;  j < MESH_COUNT; ++j) {
      mesh[j] += d;
    }
    entity->pos += d;
  }
}

// * Creates a SDL_Texture from text
SDL_Texture *render_text_as_texture(TTF_Font *font,
                                    SDL_Renderer *renderer,
                                    const char *text,
                                    SDL_Color color) {
  SDL_Surface *text_surface = stec(TTF_RenderText_Blended(font, text, color));
  SDL_Texture *text_texture = sec(SDL_CreateTextureFromSurface(renderer, text_surface));
  SDL_FreeSurface(text_surface);
  return text_texture;
}

SDL_Rect get_entity_dstrect(const Entity entity) {
  SDL_Rect dstrect = {
      entity.texbox.x + entity.pos.x, entity.texbox.y + entity.pos.y,
      entity.texbox.w, entity.texbox.h};
  return dstrect;
}

SDL_Rect get_entity_htibox(const Entity entity) {
  SDL_Rect hitbox = {
      entity.hitbox.x + entity.pos.x, entity.hitbox.y + entity.pos.y,
      entity.hitbox.w, entity.hitbox.h};
  return hitbox;
}

void render_entity(SDL_Renderer *renderer, const Entity entity) {
  const SDL_Rect entity_dstrect = get_entity_dstrect(entity);
  const SDL_RendererFlip flip = entity.dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
  render_animat(renderer, *entity.current, entity_dstrect, flip);
}

void update_entity(Entity *entity, Vec2i gravity, Uint64 dt) {
  // * Add gravity to player velocity
  entity->vel += gravity;
  entity->pos += entity->vel;

  // * Resolve entity collision
  resolve_entity_collision(entity);

  update_animat(&entity->walking, dt);
}

void render_texture(SDL_Texture *texture, SDL_Renderer *renderer, Vec2i pos) {
  int w, h;
  sec(SDL_QueryTexture(texture, nullptr, nullptr, &w, &h));
  SDL_Rect srcrect = {0, 0, w, h};
  SDL_Rect dstrect = {pos.x, pos.y, w, h};
  sec(SDL_RenderCopy(renderer, texture, &srcrect, &dstrect));
}

const size_t DIGITS_COUNT = 120;
SDL_Texture *digits_textures[DIGITS_COUNT];

void displayf(SDL_Renderer *renderer,
             TTF_Font *font,
             SDL_Color color,
             Vec2i pos,
             const char *format, ...)
{
  va_list args;
  va_start(args, format);

  char text[256];
  vsnprintf(text, sizeof(text), format, args);
  
   // * Creates a SDL_Texture from text
   SDL_Texture *texture = render_text_as_texture(font, renderer, text, color);
   // * Render texture onto screen
  render_texture(texture, renderer, pos);
  SDL_DestroyTexture(texture);
  
  va_end(args);
}

enum class Debug_Draw_State {
  Idle,
  Create,
  Delete
};

Animat load_spritesheet_animat(SDL_Renderer *renderer,
                               size_t frame_count,
                               uint32_t frame_duration,
                               const char *spritsheet_filepath)
{
  Animat result = {
    .frames = new Sprite[frame_count],
    .frames_count = frame_count,
    .frame_duration = frame_duration
  };

  // * Load the SDL_Texture from png file
  SDL_Texture *spritesheet_texture = load_texture_from_png(renderer, spritsheet_filepath);

  // * Get the texture width & height
  int spritesheet_w = 0, spritesheet_h = 0;
  SDL_QueryTexture(spritesheet_texture, nullptr, nullptr, &spritesheet_w, &spritesheet_h);
  
  // * Create Sprites from the texture information
  int sprit_w = spritesheet_w / (int) frame_count;
  int sprit_h = spritesheet_h; // * We only handle horizontal sprites

  for(int i = 0; i < (int)frame_count; ++i) {
    result.frames[i].rect = {
      .x = i * sprit_w,
      .y = 0,
      .w = sprit_w,
      .h = sprit_h};
    result.frames[i].texture = spritesheet_texture;
  }

  return result;
}

enum class Projectile_State {
  Ded = 0,
  Active,
  Poof
};

struct Projectile {
  Projectile_State state;
  Vec2i pos;
  Vec2i vel;
  Animat poof_animat;
  Animat active_animat;
  Entity_Dir dir; 
};

const size_t projectiles_count = 69;
Projectile projectiles[projectiles_count] = {};

void init_projectiles(Animat active_animat, Animat poof_animat) {
  for (size_t i = 0; i < projectiles_count; ++i) {
    projectiles[i].active_animat = active_animat;
    projectiles[i].poof_animat = poof_animat;
  }
}

// * Finds the first Ded projectile and spwans that
void spwan_projectile(Vec2i pos, Vec2i vel, Entity_Dir dir) {
  for(size_t i = 0; i < projectiles_count; ++i) {
    if(projectiles[i].state == Projectile_State::Ded) {
      projectiles[i].state = Projectile_State::Active;
      projectiles[i].pos = pos;
      projectiles[i].vel = vel;
      projectiles[i].dir = dir;
      return;
    }
  } 
}

// * Renders all the active projectiles
void render_projectiles(SDL_Renderer *renderer) {
  for(size_t i = 0; i < projectiles_count; ++i) {
    const SDL_RendererFlip flip = projectiles[i].dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    switch (projectiles[i].state)
    {
      case Projectile_State::Active: { // * active animation
        render_animat(renderer,
                      projectiles[i].active_animat,
                      projectiles[i].pos, flip);
      } break;
      case Projectile_State::Poof: { // * poof animation;
        render_animat(renderer,
                      projectiles[i].poof_animat,
                      projectiles[i].pos, flip);
      } break;
      case Projectile_State::Ded:
        break;
      default:
        break;
    }
  }
}

void update_projectiles(Uint64 dt) {
  for(size_t i = 0; i < projectiles_count; ++i) {
    switch (projectiles[i].state)
    {
    case Projectile_State::Active: { // * update active animation
      update_animat(&projectiles[i].active_animat, dt);

      // * Update the projectile position
      if(projectiles[i].state != Projectile_State::Ded) {
        projectiles[i].pos += projectiles[i].vel;
      }

      // * If projectile hit the tile then switch to poof animation
      if(!is_tile_empty(projectiles[i].pos / TILE_SIZE) || !is_tile_inbounds(projectiles[i].pos / TILE_SIZE)) {
        projectiles[i].state = Projectile_State::Poof; 
        projectiles[i].poof_animat.frame_current = 0;
      }
    } break;
    case Projectile_State::Poof: { // * poof animation
      update_animat(&projectiles[i].poof_animat, dt);
      if(projectiles[i].poof_animat.frame_current == projectiles[i].poof_animat.frames_count - 1) {
        projectiles[i].state = Projectile_State::Ded;  
      }
    } break;
    default:
      break;
    }

  }  
}

void dump_level() {
  std::printf("{\n");
  for (int y = 0; y < LEVEL_HEIGHT; ++y) {
    std::printf("{");
    for (int x = 0; x < LEVEL_WIDTH; ++x) {
      switch (level[y][x])
      {

      case Tile::Empty: {
        std::printf("Tile::Empty, ");
      } break;

      case Tile::Wall: {
        std::printf("Tile::Wall, ");
      } break;
      default:
        break;
      }
    }
    std::printf("},\n");
    std::printf("\n");
  }
  std::printf("}\n");
}

int main(void) {
  sec(SDL_Init(SDL_INIT_VIDEO));

  // * Initialize the SDL Window
  SDL_Window *window = sec(SDL_CreateWindow(
      "Game",
      0, 0,
      SCREEN_WIDTH, SCREEN_HEIGHT,
      SDL_WINDOW_RESIZABLE));

  // * Initialize the SDL Renderer
  SDL_Renderer *renderer = sec(SDL_CreateRenderer(
      window, -1,
      SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  // load font.ttf at size 16 into font
  stec(TTF_Init());
  TTF_Font *font = stec(TTF_OpenFont("assets/Comic-Sans-MS.ttf", 24));

  // * Tile Texture
  SDL_Texture *tileset_texture =
      load_texture_from_png(renderer, TILES_FILEPATH);
  Sprite ground_grass_texture = {
    .rect = {120, 128, 32, 32},
    .texture = tileset_texture};
  Sprite ground_texture = {
      .rect = {120, 128 + 10, 22, 22},
      .texture = tileset_texture};

  // * Player Texture
  const int walking_frame_count = 4, walking_frame_duration = 100;
  Animat walking = load_spritesheet_animat(renderer, walking_frame_count, walking_frame_duration, WALKING_FILEPATH);

  SDL_Rect texbox = {
      -(PLAYER_TEXBOX_SIZE / 2), -(PLAYER_TEXBOX_SIZE / 2), PLAYER_TEXBOX_SIZE, PLAYER_TEXBOX_SIZE};

  SDL_Rect hitbox = {
      -(PLAYER_HITBOX_SIZE / 2), -(PLAYER_HITBOX_SIZE / 2), PLAYER_HITBOX_SIZE - 10, PLAYER_HITBOX_SIZE};

  // * Player Idle Animation
  Animat idle = {
      .frames = walking.frames + 2, // * 3rd frame
      .frames_count = 1,
      .frame_current = 0,
      .frame_duration = 100,
      .frame_cooldown = 0};

  // * Define Player
  Entity player = { 
    .texbox = texbox, 
    .hitbox = hitbox,
    .walking = walking,
    .idle = idle,
    .current = &player.idle
 };

  // * Define Enemy
  Entity supposed_enemy = { 
    .texbox = texbox, 
    .hitbox = hitbox,
    .walking = walking,
    .idle = idle,
    .current = &supposed_enemy.idle
 };
 supposed_enemy.pos = vec2(100, 0);

  // * Initialize the projectiles animats
  Animat plasma_pop_animat = load_spritesheet_animat(renderer, 5, 200, PROJECTILE_SPARK_FILEPATH);
  Animat plasma_poof_animat = load_spritesheet_animat(renderer, 4, 200, PROJECTILE_DESTORY_FILEPATH);
  init_projectiles(plasma_pop_animat, plasma_poof_animat);

  const SDL_Rect level_boundary = {0, 0, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE};
  const int COLLISION_PROBE_SIZE = 10;
  Vec2i mouse_position = {};
  SDL_Rect collision_probe = {}, tile_rect = {};
  Debug_Draw_State state = Debug_Draw_State::Idle;
  
  const Vec2i gravity = vec2(0, 1);
  uint64_t fps = 0;
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
            player.vel.y = -20;
          } break;
          case SDLK_l: {
            quit = true;
          } break;
          case SDLK_e: {
            if(player.dir == Entity_Dir::Right) {
              spwan_projectile(player.pos, vec2(4, 0), player.dir);
            } else {
              spwan_projectile(player.pos, vec2(-4, 0), player.dir);
            }
          } break;
          case SDLK_q: {
            debug = !debug;
          } break;
          case SDLK_r: {
            player.vel.y = 0;
            player.pos = vec2(0, 0);
          } break;

          default:
            break;
          }
        } break;
        case SDL_MOUSEMOTION: {
          Vec2i p = {event.motion.x, event.motion.y};
          resolve_point_collision(&p);

          collision_probe = {
              p.x - COLLISION_PROBE_SIZE, p.y - COLLISION_PROBE_SIZE,
              COLLISION_PROBE_SIZE * 2, COLLISION_PROBE_SIZE * 2};
              
          tile_rect = {
              (event.motion.x / TILE_SIZE) * TILE_SIZE,
              (event.motion.y / TILE_SIZE) * TILE_SIZE,
              TILE_SIZE, TILE_SIZE};

          mouse_position = {event.motion.x, event.motion.y};

          Vec2i tile = vec2(event.motion.x, event.motion.y) / TILE_SIZE;
          switch(state) {
            case Debug_Draw_State::Idle: {
            } break;
            case Debug_Draw_State::Create: {
              if (is_tile_inbounds(tile))
              level[tile.y][tile.x] = Tile::Wall;
            } break;
            case Debug_Draw_State::Delete: {
              if (is_tile_inbounds(tile))
                level[tile.y][tile.x] = Tile::Empty;
            } break;
            default: {}
          }

        } break;
        case SDL_MOUSEBUTTONDOWN: {
          if(debug) {
            Vec2i tile = vec2(event.motion.x, event.motion.y) / TILE_SIZE;
            if(is_tile_inbounds(tile)) {
              if(level[tile.y][tile.x] == Tile::Empty) {
                state = Debug_Draw_State::Create;
                level[tile.y][tile.x] = Tile::Wall;
              }
              else {
                state = Debug_Draw_State::Delete;
                level[tile.y][tile.x] = Tile::Empty;
              }
            }
          }
        } break;
        case SDL_MOUSEBUTTONUP: {
          state = Debug_Draw_State::Idle; 
        } break;
      }
    }

    // * Update state
    if (keyboard[SDL_SCANCODE_D]) {
      player.vel.x = PLAYER_SPEED;
      player.current = &player.walking;
      player.dir = Entity_Dir::Right;
    } else if (keyboard[SDL_SCANCODE_A]) {
      player.vel.x = -PLAYER_SPEED;
      player.current = &player.walking;
      player.dir = Entity_Dir::Left;
    } else {
      player.current = &player.idle;
      player.vel.x = 0;
    }

    // * Render state
    sec(SDL_SetRenderDrawColor(renderer, COLOR_BLACK));
    sec(SDL_RenderClear(renderer));
    render_level(renderer, ground_grass_texture, ground_texture);
    render_entity(renderer, player);
    render_entity(renderer, supposed_enemy);
    render_projectiles(renderer);

    // * Show player hitbox
    if(debug) {
      sec(SDL_SetRenderDrawColor(renderer, COLOR_RED));
      
      SDL_Rect entity_dstrect = get_entity_dstrect(player);
      sec(SDL_RenderDrawRect(renderer, &entity_dstrect));

      sec(SDL_RenderFillRect(renderer, &collision_probe));
      sec(SDL_RenderDrawRect(renderer, &tile_rect));
      sec(SDL_RenderDrawRect(renderer, &level_boundary));

      const uint64_t t = SDL_GetTicks64() - begin;
      const uint64_t fps_snapshot = t ? 1000 / t : 0;
      fps = (fps + fps_snapshot) / 2;
      
      const size_t gap = 35;
      displayf(renderer,
               font,
               {255, 0, 0, 255},
               {0, gap},
               "Mouse Position: (%d %d)", mouse_position.x, mouse_position.y);
      displayf(renderer,
               font,
               {255, 255, 0, 255},
               {0, gap * 2},
               "Collision Probe: (%d %d)", collision_probe.x, collision_probe.y);

      sec(SDL_SetRenderDrawColor(renderer, COLOR_YELLOW));
      SDL_Rect hitbox = get_entity_htibox(player);
      sec(SDL_RenderDrawRect(renderer, &hitbox));
    }


    SDL_RenderPresent(renderer);
    const Uint64 dt = SDL_GetTicks64() - begin;


    update_entity(&player, gravity, dt);
    update_entity(&supposed_enemy, gravity, dt);
    update_projectiles(dt);
  }

  SDL_Quit();
  // dump_level();
  return 0;
}