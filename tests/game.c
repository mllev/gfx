#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

#include "../src/geometry.h"

void draw_frame ()
{
  gfx_matrix_mode(GFX_VIEW_MATRIX);
  gfx_identity();
  gfx_translate(0, -10, 0);
  gfx_rotate(1, 0, 0, -GFX_PI / 5);

  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(10, 0, 25);
  gfx_scale(3, 6, 3);
  gfx_translate(-0.5, -0.5, -0.5);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12, NULL, 12);
  gfx_draw_arrays(0, -1);

  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(0, -3, 25);
  gfx_scale(25, 3, 25);
  gfx_translate(-0.5, -0.5, -0.5);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12, NULL, 12);
  gfx_draw_arrays(0, -1);
}

int main (void) {
  window_t window;
  int width = 960, height = 540;
  unsigned int* buf;
  float *zbuf;
  float fov = 70;
  unsigned int frame, start;
  char debug_string[50];

  buf = (unsigned int *)malloc((width * height) * sizeof(unsigned int));
  zbuf = (float *)malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  window_open(&window, "Game", width, height);

  gfx_init();
  gfx_bind_render_target(buf, width, height);
  gfx_bind_depth_buffer(zbuf);

  while (!window.quit) {
    gfx_matrix_mode(GFX_PROJECTION_MATRIX);
    gfx_perspective(fov, (float)width / (float)height, 1, 1000.0);

    start = SDL_GetTicks();

    draw_frame();

    frame = SDL_GetTicks() - start;

    sprintf(debug_string, "frame: %dms", frame);

    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);
    gfx_draw_text_8x8(ascii, "Welcome to game.", 16, width / 2 - 64, height / 2 - 4);

    window_update(&window, buf);

    gfx_clear();
  }

  window_close(&window);
  free(buf);
  free(zbuf);

  return 0;
}
