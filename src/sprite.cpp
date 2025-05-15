
// * ####################
// * Sprites
// * ####################

struct Sprite {
  SDL_Rect rect;
  SDL_Texture *texture;
};

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

// * ####################
// * Animat
// * ####################

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
  assert(animat);
  
  if (dt < animat->frame_cooldown) {
    animat->frame_cooldown -= dt;
  }
  else {
    animat->frame_current = (animat->frame_current + 1) % animat->frames_count;
    animat->frame_cooldown = animat->frame_duration;
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