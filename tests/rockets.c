#include <stdio.h>
#include <time.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

float white_color[] = { 1.0, 1.0, 1.0 };
float red_color[] = { 1.0, 0.0, 0.0 };
float blue_color[] = { 0.0, 0.0, 1.0 };

typedef struct _vec2 Vec2;
typedef struct _vec3 Vec3;
typedef struct _rocket Rocket;
typedef struct _particles Particles;

/* vector stuff */
struct _vec2 { float x, y; };
struct _vec3 { float x, y, z; };

float vec2_length(Vec2* v)
{
  return sqrt(v->x * v->x + v->y * v->y);
}

/* particle stuff */
#define PARTICLE_MAX 500

struct _particles {
  struct {
    Vec2 position;
    Vec2 velocity;
    Vec2 scale;
    float duration;
  } data[PARTICLE_MAX];

  int count;
  int start;
};

void Particles_init(Particles *p)
{
  int i;

  p->count = 0;
  p->start = 0;

  for (i = 0; i < PARTICLE_MAX; i++) {
    p->data[i].duration = 0.0;
  }
}

void Particles_add(Particles *p, Vec2 *position, Vec2 *velocity, Vec2 *scale, float duration)
{
  int i;

  for (i = 0; i < PARTICLE_MAX; i++) {
    if (p->data[i].duration <= 0) {
      break;
    }
  }

  if (i == 50) {
    return;
  }

  p->data[i].scale.x = scale->x;
  p->data[i].scale.y = scale->y;

  p->data[i].velocity.x = velocity->x;
  p->data[i].velocity.y = velocity->y;

  p->data[i].position.x = position->x + p->data[i].velocity.x;
  p->data[i].position.y = position->y + p->data[i].velocity.y;

  p->data[i].duration = duration;
}

typedef struct camera Camera;

struct camera {
  float max_height;
  float current_height;
  float min_height;
};

void Particles_render(Particles *p, Vec2 *frame)
{
  int i;

  for (i = 0; i < PARTICLE_MAX; i++) {
    if (p->data[i].duration <= 0) {
      continue;
    }

    gfx_matrix_mode(GFX_MODEL_MATRIX);
    gfx_identity();
    gfx_translate(p->data[i].position.x, p->data[i].position.y, 0);
    gfx_scale(p->data[i].scale.x, p->data[i].scale.y, 0);
    gfx_bind_primitive(GFX_PRIMITIVE_QUAD);
    gfx_bind_attr(GFX_ATTR_RGB, white_color);
    gfx_draw_arrays(0, -1);

    if (frame) {
      p->data[i].position.x += (p->data[i].velocity.x + frame->x);
      p->data[i].position.y += (p->data[i].velocity.y + frame->y);
    } else {
      p->data[i].position.x += (p->data[i].velocity.x);
      p->data[i].position.y += (p->data[i].velocity.y);
    }

    p->data[i].duration -= 0.01;
  }
}

/* rocket stuff */
struct _rocket {
  Vec2 position;
  Vec2 forward;
  Vec2 velocity;

  Particles bullets;
  Particles flame;

  int is_slowing;

  float speed;
  float rotation;
  float max_speed;
};

void Rocket_rotate(Rocket *r, float a)
{
  float s, c, x, y;

  r->rotation += a;
  r->forward.x = 0.0;
  r->forward.y = 1.0;

  s = sin(r->rotation);
  c = cos(r->rotation);
  
  x = c * r->forward.x - s * r->forward.y;
  y = s * r->forward.x + c * r->forward.y;
  
  r->forward.x = x;
  r->forward.y = y;
}

void Rocket_move_forward(Rocket *r)
{
  r->velocity.x += r->forward.x * r->speed;
  r->velocity.y += r->forward.y * r->speed;

  if (vec2_length(&r->velocity) > r->max_speed) {
    r->velocity.x -= r->forward.x * r->speed;
    r->velocity.y -= r->forward.y * r->speed;
  }

  { /* rocket trail */
    Vec2 direction = { 0.0, 0.0 };
    Vec2 scale = { 0.2, 0.2 };

    Particles_add(&r->flame, &r->position, &direction, &scale, 0.5);
  }
}

void Rocket_fire(Rocket *r)
{
  Vec2 scale = { 0.2, 0.2 };
  Particles_add(&r->bullets, &r->position, &r->forward, &scale, 0.5);
}

void Rocket_update(Rocket *r)
{
  int i;

  r->position.x += r->velocity.x;
  r->position.y += r->velocity.y;

  if (r->is_slowing) {
    r->velocity.x *= 0.96;
    r->velocity.y *= 0.96;
  }
}

void Rocket_render(Rocket *r)
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(r->position.x, r->position.y, 0);
  gfx_rotate(0, 0, 1, r->rotation);
  gfx_bind_primitive(GFX_PRIMITIVE_TRIANGLE);
  gfx_bind_attr(GFX_ATTR_RGB, white_color);
  gfx_draw_arrays(0, -1);

  /* clean: bullets require the ships current velocity */
  Particles_render(&r->bullets, &r->velocity);
  Particles_render(&r->flame, NULL);
}

void Rocket_init(Rocket *r)
{
  r->forward.x = 0.0;
  r->forward.y = 1.0;

  r->velocity.x = 0.0;
  r->velocity.y = 0.0;

  r->position.x = 0.0;
  r->position.y = 0.0;

  r->max_speed = 10.0;
  r->is_slowing = 0;
  r->speed = 0.1;
  r->rotation = 0.0;

  Particles_init(&r->bullets);
  Particles_init(&r->flame);
}

Particles Explosions;

void init_explosions()
{
  int i;

  Particles_init(&Explosions);

  for (i = 0; i < 20; i++) {
    Vec2 direction;
    Vec2 scale = { 1, 1 };
    Vec2 position = { 0.0, 0.0 };

    direction.x = ((float)rand() / (float)RAND_MAX * 2.0 - 1.0) * 0.1;
    direction.y = ((float)rand() / (float)RAND_MAX * 2.0 - 1.0) * 0.1;

    Particles_add(&Explosions, &position, &direction, &scale, (float)rand() / (float)RAND_MAX);
  }
}

void render_explosions()
{
  Particles_render(&Explosions, NULL);
}

Vec3 background[1000];

int main (void) {
  window_t window;
  int width = 1280, height = 720;
  unsigned int* framebuffer;
  float* depthbuffer;
  unsigned int frame, start;
  int i;
  Camera camera;
  Rocket rocket;
  char debug_string[50];

  int enter_detected = 0;

  depthbuffer = (float *)malloc((width * height) * sizeof(float));
  framebuffer = (unsigned int *)malloc((width * height) * sizeof(unsigned int));

  if (!framebuffer) return 1;

  window_open(&window, "rockets", width, height);

  srand(time(NULL));

  gfx_init();
  gfx_bind_render_target(framebuffer, width, height);
  gfx_bind_depth_buffer(depthbuffer);
  gfx_set_projection(70, (float)width / (float)height, 1);

  init_explosions();

  for (i = 0; i < 1000; i++) {
    background[i].x = (float)(rand() % 500);
    background[i].y = (float)(rand() % 500);
    background[i].z = (float)(rand() % 100);
  }

  Rocket_init(&rocket);

  camera.current_height = 0;
  camera.min_height = 15;
  camera.max_height = 150;

  while (!window.quit) {
    start = SDL_GetTicks();

    /* event handling */
    if (window.keys.w) {
      Rocket_move_forward(&rocket);
      rocket.is_slowing = 0;
    } else {
      rocket.is_slowing = 1;
    }

    if (window.keys.left) {
      Rocket_rotate(&rocket, 0.1);
    }

    if (window.keys.right) {
      Rocket_rotate(&rocket, -0.1);
    }

    if (window.keys.enter) {
      if (!enter_detected) {
        Rocket_fire(&rocket);
        enter_detected = 1;
      }
    } else {
      enter_detected = 0;
    }

    { /* update camera height */
      float current_rocket_speed = vec2_length(&rocket.velocity);
      float frac = current_rocket_speed / rocket.max_speed;
      camera.current_height = (frac * frac) * (camera.max_height - camera.min_height) + camera.min_height;
    }

    gfx_matrix_mode(GFX_VIEW_MATRIX);
    gfx_identity();
    gfx_translate(-rocket.position.x, -rocket.position.y, camera.current_height);

    Rocket_render(&rocket);
    Rocket_update(&rocket);

    render_explosions();

    /* render background */
    for (i = 0; i < 1000; i++) {
      gfx_matrix_mode(GFX_MODEL_MATRIX);
      gfx_identity();
      gfx_translate(background[i].x, background[i].y, background[i].z);
      gfx_scale(5, 5, 0);
      gfx_bind_primitive(GFX_PRIMITIVE_QUAD);
      gfx_bind_attr(GFX_ATTR_RGB, red_color);
      gfx_draw_arrays(0, -1);
    }

    frame = SDL_GetTicks() - start;
    sprintf(debug_string, "frame: %dms", frame);
    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);

    window_render(&window, framebuffer);
    gfx_clear();
  }

  window_close(&window);
  free(framebuffer);

  return 0;
}
