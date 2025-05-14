
enum class Tile
{
  Empty = 0,
  Wall
};

const int TILE_SIZE = 64;

const int LEVEL_WIDTH = 10;
const int LEVEL_HEIGHT = 10;
const SDL_Rect level_boundary = {0, 0, LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE};

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


static inline
bool is_tile_inbounds(Vec2i p) {
  return 0 <= p.x && p.x < LEVEL_WIDTH && 0 <= p.y && p.y < LEVEL_HEIGHT;
}

bool is_tile_empty(Vec2i p) {
  return !is_tile_inbounds(p) || level[p.y][p.x] == Tile::Empty;
}

void render_level(SDL_Renderer *renderer, Sprite top_ground_texture, Sprite bottom_ground_texture) {
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
