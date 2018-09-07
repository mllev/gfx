#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

#define BMPREAD_IMPLEMENT
#include "../deps/bmpread.h"

#include "../src/geometry.h"

struct {
  struct {
    float x, y, z;
    float speed;
  } camera;

  struct {
    float x, y, z;
    float speed;
    int is_moving_left;
    int is_moving_right;
    int is_moving_forwards;
    int is_moving_backwards;
  } player;

} STATE;

float blue_color[]  = { 0.349, 0.647, 0.847, };
float white_color[] = { 1.0, 1.0, 1.0 };
float brown_color[] = { 0.573, 0.416, 0.176 };

void draw_board ()
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_scale(30, 1, 30);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, blue_color);
  gfx_draw_arrays(0, -1);
}

void draw_exit ()
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, brown_color);

  gfx_identity();
  gfx_translate(9, 1, 30);
  gfx_scale(3, 6, 3);
  gfx_draw_arrays(0, -1);

  gfx_identity();
  gfx_translate(15, 1, 30);
  gfx_scale(3, 6, 3);
  gfx_draw_arrays(0, -1);

  gfx_identity();
  gfx_translate(12, 4, 30);
  gfx_scale(3, 3, 3);
  gfx_draw_arrays(0, -1);
}

void draw_player ()
{
  if (STATE.player.is_moving_forwards) {
    STATE.player.z += STATE.player.speed;
    if (STATE.player.z > 30 - 3) STATE.player.z = 30 - 3;
    if (fmod(STATE.player.z, 3) == 0) {
      STATE.player.is_moving_forwards = 0;
    }
  }

  if (STATE.player.is_moving_backwards) {
    STATE.player.z -= STATE.player.speed;
    if (STATE.player.z < 0) STATE.player.z = 0;
    if (fmod(STATE.player.z, 3) == 0) {
      STATE.player.is_moving_backwards = 0;
    }
  }

  if (STATE.player.is_moving_right) {
    STATE.player.x += STATE.player.speed;
    if (STATE.player.x > 30 - 3) STATE.player.x = 30 - 3;
    if (fmod(STATE.player.x, 3) == 0) {
      STATE.player.is_moving_right = 0;
    }
  }

  if (STATE.player.is_moving_left) {
    STATE.player.x -= STATE.player.speed;
    if (STATE.player.x < 0) STATE.player.x = 0;
    if (fmod(STATE.player.x, 3) == 0) {
      STATE.player.is_moving_left = 0;
    }
  }

  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(STATE.player.x, STATE.player.y, STATE.player.z);
  gfx_scale(3, 3, 3);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, white_color);
  gfx_draw_arrays(0, -1);
}

void draw_frame ()
{
  gfx_matrix_mode(GFX_VIEW_MATRIX);
  gfx_identity();
  gfx_rotate(1, 0, 0, -1.01);
  gfx_translate(-STATE.camera.x, -STATE.camera.y, -STATE.camera.z);

  draw_player();
  draw_board();
  draw_exit();
}

void init_game ()
{
  memset(&STATE, 0, sizeof(STATE));

  STATE.camera.y = 30;
  STATE.camera.x = 15;
  STATE.camera.z = -7;
  STATE.camera.speed = 0.1;

  STATE.player.y = 1;
  STATE.player.speed = 0.5;
}

unsigned int* load_texture (const char *file)
{
  int x, y, w = 64, h = 64;
  bmpread_t bmp1;
  unsigned int *texture;
  if (bmpread(file, 0, &bmp1)) {
    texture = malloc(sizeof(unsigned int) * bmp1.width * bmp1.height);
    for (x = 0; x < w; x++) {
      for (y = 0; y < h; y++) {
        int idx = w * y + x;
        int r = bmp1.rgb_data[idx*3];
        int g = bmp1.rgb_data[idx*3+1];
        int b = bmp1.rgb_data[idx*3+2];

        texture[idx] = (255 << 24) | (r << 16) | (g << 8) | b;
      }
    }
    return texture;
  } else {
    printf("No file.\n");
    return NULL;
  }
}

int main (void) {
  window_t window;
  int width = 1280, height = 720;
  unsigned int* buf;
  float *zbuf;
  float fov = 60;
  unsigned int frame, start;
  char debug_string[50];

  buf = (unsigned int *)malloc((width * height) * sizeof(unsigned int));
  zbuf = (float *)malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  window_open(&window, "Game", width, height);

  gfx_init();
  gfx_bind_render_target(buf, width, height);
  gfx_bind_depth_buffer(zbuf);
  gfx_set_projection(fov, (float)width / (float)height, 1);

  init_game();

  while (!window.quit) {
    start = SDL_GetTicks();

    if (window.keys.w) {
      STATE.camera.z += STATE.camera.speed;
    }

    if (window.keys.s) {
      STATE.camera.z -= STATE.camera.speed;
    }

    if (window.keys.a) {
      STATE.camera.x -= STATE.camera.speed;
    }

    if (window.keys.d) {
      STATE.camera.x += STATE.camera.speed;
    }

    if (window.keys.up && !STATE.player.is_moving_forwards) {
      STATE.player.is_moving_forwards = 1;
    }

    if (window.keys.down && !STATE.player.is_moving_backwards) {
      STATE.player.is_moving_backwards = 1;
    }

    if (window.keys.left && !STATE.player.is_moving_left) {
      STATE.player.is_moving_left = 1;
    }

    if (window.keys.right && !STATE.player.is_moving_right) {
      STATE.player.is_moving_right = 1;
    }

    draw_frame();

    frame = SDL_GetTicks() - start;

    sprintf(debug_string, "frame: %dms", frame);

    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);
    window_update(&window, buf);
    gfx_clear();
  }

  window_close(&window);
  free(buf);
  free(zbuf);

  return 0;
}
