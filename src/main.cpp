
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TILES_FILEPATH "assets/sprites/fantasy_tiles.png"
#define WALKING_FILEPATH "assets/sprites/walking-12px.png"
#define PROJECTILE_SPARK_FILEPATH "assets/sprites/spark-sheet.png"
#define PROJECTILE_DESTORY_FILEPATH "assets/sprites/destroy-sheet.png"

#define COLOR_BLACK 0x00, 0x00, 0x00, 0xff
#define COLOR_RED 0xff, 0x00, 0x00, 0xff
#define COLOR_YELLOW 0xff, 0xff, 0x00, 0xff

const int PLAYER_SPEED = 2;
const int PLAYER_TEXBOX_SIZE = 48;
const int PLAYER_HITBOX_SIZE = (PLAYER_TEXBOX_SIZE - 10);

template <typename T>
T max(T n1, T n2) {
  return n1 > n2 ? n1 : n2;
}

SDL_Rect get_entity_htibox(const Entity entity) {
  SDL_Rect hitbox = {
      entity.hitbox.x + entity.pos.x, entity.hitbox.y + entity.pos.y,
      entity.hitbox.w, entity.hitbox.h};
  return hitbox;
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
            entity_shoot(&player);
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

    entity_shoot(&supposed_enemy);

    // * Update state
    if (keyboard[SDL_SCANCODE_D]) {
      entity_move(&player, PLAYER_SPEED);
    } else if (keyboard[SDL_SCANCODE_A]) {
      entity_move(&player, -PLAYER_SPEED);
    } else {
      entity_stop(&player);
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