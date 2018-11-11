#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

float white_color[] = { 1.0, 1.0, 1.0 };
float red_color[] = { 1.0, 0.0, 0.0 };
float blue_color[] = { 0.0, 0.0, 1.0 };

/* vector stuff */
typedef struct _vec2 Vec2;
typedef struct _vec3 Vec3;

struct _vec2 { float x, y; };
struct _vec3 { float x, y, z; };

float vec2_length(Vec2* v)
{
  return sqrt(v->x * v->x + v->y * v->y);
}

/* rocket stuff */
typedef struct _rocket Rocket;

struct _rocket {
  Vec2 position;
  Vec2 forward;
  Vec2 velocity;

  struct {
    Vec2 position;
    Vec2 velocity;
  } bullets[50];

  int current_bullet_index; /* used for fast reuse of array slots */

  int num_bullets;

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
}

void Rocket_fire(Rocket *r)
{
  int index;

  if (r->num_bullets < 50) {
    index = r->num_bullets++;
  } else if (r->current_bullet_index <= 49 ) {
    index = r->current_bullet_index++;
  } else {
    index = r->current_bullet_index = 0;
  }

  r->bullets[index].velocity.x = r->forward.x;
  r->bullets[index].velocity.y = r->forward.y;
  r->bullets[index].position.x = r->position.x + r->bullets[index].velocity.x;
  r->bullets[index].position.y = r->position.y + r->bullets[index].velocity.y;
}

void Rocket_update(Rocket *r)
{
  r->position.x += r->velocity.x;
  r->position.y += r->velocity.y;

  if (r->is_slowing) {
    r->velocity.x *= 0.96;
    r->velocity.y *= 0.96;
  }
}

void Rocket_render(Rocket *r)
{
  int i;

  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(r->position.x, r->position.y, 0);
  gfx_rotate(0, 0, 1, r->rotation);
  gfx_bind_primitive(GFX_PRIMITIVE_TRIANGLE);
  gfx_bind_attr(GFX_ATTR_RGB, white_color);
  gfx_draw_arrays(0, -1);

  for (i = 0; i < r->num_bullets; i++) {
    gfx_matrix_mode(GFX_MODEL_MATRIX);
    gfx_identity();
    gfx_translate(r->bullets[i].position.x, r->bullets[i].position.y, 0);
    gfx_scale(0.2, 0.2, 0);
    gfx_bind_primitive(GFX_PRIMITIVE_QUAD);
    gfx_bind_attr(GFX_ATTR_RGB, white_color);
    gfx_draw_arrays(0, -1);

    r->bullets[i].position.x += (r->bullets[i].velocity.x + r->velocity.x);
    r->bullets[i].position.y += (r->bullets[i].velocity.y + r->velocity.y);
  }
}

void Rocket_init(Rocket *r)
{
  r->speed = 0.1;
  r->rotation = 0.0;
  r->forward.x = 0.0;
  r->forward.y = 1.0;
  r->velocity.x = 0.0;
  r->velocity.y = 0.0;
  r->position.x = 0.0;
  r->position.y = 0.0;
  r->num_bullets = 0;
  r->max_speed = 5.0;
  r->current_bullet_index = 0;
  r->is_slowing = 0;
}

Vec3 background[1000];

int main (void) {
  window_t window;
  int width = 1280, height = 720;
  unsigned int* framebuffer;
  float* depthbuffer;
  unsigned int frame, start;
  int i;
  float camera_z;
  Rocket rocket;
  char debug_string[50];

  int enter_detected = 0;

  depthbuffer = (float *)malloc((width * height) * sizeof(float));
  framebuffer = (unsigned int *)malloc((width * height) * sizeof(unsigned int));

  if (!framebuffer) return 1;

  window_open(&window, "rockets", width, height);

  gfx_init();
  gfx_bind_render_target(framebuffer, width, height);
  gfx_bind_depth_buffer(depthbuffer);
  gfx_set_projection(70, (float)width / (float)height, 1);

  for (i = 0; i < 1000; i++) {
    background[i].x = (float)(rand() % 500);
    background[i].y = (float)(rand() % 500);
    background[i].z = (float)(rand() % 100);
  }

  Rocket_init(&rocket);
  camera_z = 15;

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
      camera_z = (frac * frac) * (50 - 15) + 15;
    }

    gfx_matrix_mode(GFX_VIEW_MATRIX);
    gfx_identity();
    gfx_translate(-rocket.position.x, -rocket.position.y, camera_z);

    Rocket_render(&rocket);
    Rocket_update(&rocket);

    { /* render background */
      for (i = 0; i < 1000; i++) {
        gfx_matrix_mode(GFX_MODEL_MATRIX);
        gfx_identity();
        gfx_translate(background[i].x, background[i].y, background[i].z);
        gfx_scale(5, 5, 0);
        gfx_bind_primitive(GFX_PRIMITIVE_QUAD);
        gfx_bind_attr(GFX_ATTR_RGB, red_color);
        gfx_draw_arrays(0, -1);
      }
    }

    frame = SDL_GetTicks() - start;
    sprintf(debug_string, "frame: %dms", frame);
    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);

    window_update(&window, framebuffer);
    gfx_clear();
  }

  window_close(&window);
  free(framebuffer);

  return 0;
}
