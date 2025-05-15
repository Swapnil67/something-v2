enum class Entity_Dir {
  Right = 0,
  Left
};

enum class Entity_State {
  Ded = 0,
  Alive
};

struct Entity {

  Entity_State state;

  SDL_Rect texbox;
  SDL_Rect hitbox;
  Vec2i pos;
  Vec2i vel;

  Animat idle;
  Animat walking;
  Animat *current;

  Entity_Dir dir;

  int weapon_cooldown;
};

SDL_Rect get_entity_dstrect(const Entity entity) {
  SDL_Rect dstrect = {
      entity.texbox.x + entity.pos.x, entity.texbox.y + entity.pos.y,
      entity.texbox.w, entity.texbox.h};
  return dstrect;
}

void render_entity(SDL_Renderer *renderer, const Entity entity) {
  assert(renderer);
  if (entity.state == Entity_State::Ded)
    return;
  const SDL_Rect entity_dstrect = get_entity_dstrect(entity);
  const SDL_RendererFlip flip = entity.dir == Entity_Dir::Right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
  render_animat(renderer, *entity.current, entity_dstrect, flip);
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

void update_entity(Entity *entity, Vec2i gravity, Uint64 dt) {
  assert(entity);

  if (entity->state == Entity_State::Ded)
    return;

  // * Add gravity to player velocity
  entity->vel += gravity;
  entity->pos += entity->vel;

  // * Resolve entity collision
  resolve_entity_collision(entity);

  entity->weapon_cooldown -= 1;

  update_animat(&entity->walking, dt);
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

void entity_move(Entity *entity, int speed) {
  assert(entity);
  entity->vel.x = speed;

  // * Move entity in right direction
  if(speed > 0) {
    entity->dir = Entity_Dir::Right;
  } else if(speed < 0) {
    // * Move entity in left direction
    entity->dir = Entity_Dir::Left;
  }

  entity->current = &entity->walking;
}

void entity_stop(Entity *entity) {
  assert(entity);
  entity->vel.x = 0;
  entity->current = &entity->idle;
}

const int ENTITY_WEAPON_COOLDOWN = 30;

// * shoots the projectile
void entity_shoot(Entity *entity) {
  assert(entity);

  if (entity->weapon_cooldown > 0)
    return;

  if (entity->dir == Entity_Dir::Right) {
    spwan_projectile(entity->pos, vec2(4, 0));
  } else {
    spwan_projectile(entity->pos, vec2(-4, 0));
  }

  entity->weapon_cooldown = ENTITY_WEAPON_COOLDOWN;
}

const int entities_count = 69;
Entity entities[entities_count];

// * Update the list of entities
void udpate_entities(Vec2i gravity, Uint64 dt) {
  for(int i = 0; i < entities_count; ++i) {
    update_entity(&entities[i], gravity, dt);
  }
}

// * Render the list of entities
void render_entities(SDL_Renderer *renderer) {
  for(int i = 0; i < entities_count; ++i) {
    if (entities[i].state == Entity_State::Ded)
      continue;
    render_entity(renderer, entities[i]);
  }
}

