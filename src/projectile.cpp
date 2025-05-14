
// * ####################
// * Projectiles
// * ####################

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
void spwan_projectile(Vec2i pos, Vec2i vel) {
  for(size_t i = 0; i < projectiles_count; ++i) {
    if(projectiles[i].state == Projectile_State::Ded) {
      projectiles[i].state = Projectile_State::Active;
      projectiles[i].pos = pos;
      projectiles[i].vel = vel;
      return;
    }
  } 
}

// * Renders all the active projectiles
void render_projectiles(SDL_Renderer *renderer) {
  for(size_t i = 0; i < projectiles_count; ++i) {
    switch (projectiles[i].state)
    {
      case Projectile_State::Active: { // * active animation
        render_animat(renderer,
                      projectiles[i].active_animat,
                      projectiles[i].pos);
      } break;
      case Projectile_State::Poof: { // * poof animation;
        render_animat(renderer,
                      projectiles[i].poof_animat,
                      projectiles[i].pos);
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
