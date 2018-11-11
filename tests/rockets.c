#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

float white_color[] = { 1.0, 1.0, 1.0 };
float red_color[] = { 1.0, 0.0, 0.0 };
float blue_color[] = { 0.0, 0.0, 1.0 };

typedef struct _rocket Rocket;
typedef struct _vec2 Vec2;
typedef struct _vec3 Vec3;

struct _vec2 { float x, y; };
struct _vec3 { float x, y, z; };

struct _rocket {
  Vec2 position;
  Vec2 forward;
  Vec2 velocity;

  struct {
    Vec2 position;
    Vec2 velocity;
  } bullets[50];

  int num_bullets;

  float speed;
  float rotation;
};

Vec3 background[1000];

float vec2_length(Vec2* v)
{
  return sqrt(v->x * v->x + v->y * v->y);
}

void Rocket_rotate(Rocket* r, float a)
{
  float s = sin(a);
  float c = cos(a);
  
  float x = c * r->forward.x - s * r->forward.y;
  float y = s * r->forward.x + c * r->forward.y;
  
  r->forward.x = x;
  r->forward.y = y;
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
}

int main (void) {
  window_t window;
  int width = 1280, height = 720;
  unsigned int* framebuffer;
  float* depthbuffer;
  unsigned int frame, start;
  int i;
  float camera_z;
  float current_rocket_speed;
  float max_rocket_speed = 5.0;
  Rocket rocket;
  char debug_string[50];
  float rotate_amt = 0.0;
  int current_bullet_index = 0;

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
    current_rocket_speed = vec2_length(&rocket.velocity);

    if (window.keys.w) {
      rocket.velocity.x += rocket.forward.x * rocket.speed;
      rocket.velocity.y += rocket.forward.y * rocket.speed;

      if (vec2_length(&rocket.velocity) > max_rocket_speed) {
        rocket.velocity.x -= rocket.forward.x * rocket.speed;
        rocket.velocity.y -= rocket.forward.y * rocket.speed;
      }
    }

    {
      float frac = current_rocket_speed / max_rocket_speed;
      camera_z = (frac * frac) * (50 - 15) + 15;
    }

    if (window.keys.left) {
      rocket.rotation += 0.1;

      rocket.forward.x = 0.0;
      rocket.forward.y = 1.0;

      Rocket_rotate(&rocket, rocket.rotation);
    }

    if (window.keys.right) {
      rocket.rotation -= 0.1;

      rocket.forward.x = 0.0;
      rocket.forward.y = 1.0;

      Rocket_rotate(&rocket, rocket.rotation);
    }

    if (window.keys.enter) {
      if (!enter_detected) {
        int index;

        if (rocket.num_bullets < 50) {
          index = rocket.num_bullets++;
        } else if (current_bullet_index <= 49 ) {
          index = current_bullet_index++;
        } else {
          index = current_bullet_index = 0;
        }

        rocket.bullets[index].velocity.x = rocket.forward.x;
        rocket.bullets[index].velocity.y = rocket.forward.y;
        rocket.bullets[index].position.x = rocket.position.x + rocket.bullets[index].velocity.x;
        rocket.bullets[index].position.y = rocket.position.y + rocket.bullets[index].velocity.y;

        enter_detected = 1;
      }
    } else {
      enter_detected = 0;
    }

    gfx_matrix_mode(GFX_VIEW_MATRIX);
    gfx_identity();
    gfx_translate(-rocket.position.x, -rocket.position.y, camera_z);

    { /* render rocket */
      gfx_matrix_mode(GFX_MODEL_MATRIX);
      gfx_identity();
      gfx_translate(rocket.position.x, rocket.position.y, 0);
      gfx_rotate(0, 0, 1, rocket.rotation);
      gfx_bind_primitive(GFX_PRIMITIVE_TRIANGLE);
      gfx_bind_attr(GFX_ATTR_RGB, white_color);
      gfx_draw_arrays(0, -1);
    }

    { /* render bullets */
      for (i = 0; i < rocket.num_bullets; i++) {
        gfx_matrix_mode(GFX_MODEL_MATRIX);
        gfx_identity();
        gfx_translate(rocket.bullets[i].position.x, rocket.bullets[i].position.y, 0);
        gfx_scale(0.2, 0.2, 0);
        gfx_bind_primitive(GFX_PRIMITIVE_QUAD);
        gfx_bind_attr(GFX_ATTR_RGB, white_color);
        gfx_draw_arrays(0, -1);

        rocket.bullets[i].position.x += (rocket.bullets[i].velocity.x + rocket.velocity.x);
        rocket.bullets[i].position.y += (rocket.bullets[i].velocity.y + rocket.velocity.y);
      }
    }

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

    rocket.position.x += rocket.velocity.x;
    rocket.position.y += rocket.velocity.y;

    if (!window.keys.w) {
      rocket.velocity.x *= 0.96;
      rocket.velocity.y *= 0.96;
    }

    frame = SDL_GetTicks() - start;
    sprintf(debug_string, "frame: %dms", frame);
    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);

    window_update(&window, framebuffer);
    gfx_clear();

    rotate_amt += 0.01;
  }

  window_close(&window);
  free(framebuffer);

  return 0;
}
