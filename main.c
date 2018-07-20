#include <stdio.h>

#include "font.h"

#define WINDOW_IMPL
#include "window.h"

/* 
options:
  GFX_DEBUG
  GFX_IMPLEMENT
  GFX_USE_MALLOC
*/

#define GFX_DEBUG
#define GFX_IMPLEMENT

#include "gfx.h"

float cube_vertices[] = {
  0.0, 0.0, 0.0,
  0.0, 0.0, 1.0,
  0.0, 1.0, 0.0,
  0.0, 1.0, 1.0,
  1.0, 0.0, 0.0,
  1.0, 0.0, 1.0,
  1.0, 1.0, 0.0,
  1.0, 1.0, 1.0
};

int cube_indices[] = {
  0, 6, 4,
  0, 2, 6,
  0, 3, 2,
  0, 1, 3,
  2, 7, 6,
  2, 3, 7,
  4, 6, 7,
  4, 7, 5,
  0, 4, 5,
  0, 5, 1,
  1, 5, 7,
  1, 7, 3
};

float cube_colors[] = {
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0
};

int main (int argc, char **argv)
{
  int width = 1280;
  int height = 720;
  unsigned int* buf;
  float *zbuf;
  window_t window;

  float cam_y_rotate = 0;
  float rotate_amt = 0;

  buf = malloc((width * height) * sizeof(unsigned int));
  zbuf = malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  window_open(&window, "GFX", width, height);

  gfx_init();
  gfx_bind_render_target(buf, width, height);
  gfx_bind_depth_buffer(zbuf);

  gfx_matrix_mode(GFX_PROJECTION_MATRIX);
  gfx_perspective(70, (float)width / (float)height, .1, 1000.0);

  gfx_matrix_mode(GFX_VIEW_MATRIX);
  gfx_identity();

  while (!window.quit) {
    gfx_matrix_mode(GFX_VIEW_MATRIX);

    if (window.keys.left)  gfx_translate(1.0, 0, 0);
    if (window.keys.right) gfx_translate(-1.0, 0, 0);
    if (window.keys.up)    gfx_translate(0, 0, -1.0);
    if (window.keys.down)  gfx_translate(0, 0, 1.0);

    gfx_bind_arrays(cube_vertices, 8, cube_indices, 12, cube_colors, 12);

    gfx_matrix_mode(GFX_MODEL_MATRIX);
    gfx_identity();
    gfx_translate(0, 0, 20);
    gfx_rotate(0, 0, 1, rotate_amt);
    gfx_rotate(0, 1, 0, rotate_amt);
    gfx_scale(10);
    gfx_translate(-0.5, -0.5, -0.5);

    gfx_draw_arrays(0, -1);
    gfx_draw_text_8x8(ascii, "Hello there!", 12, 0, 0);

    window_update(&window, buf);

    gfx_clear();

    rotate_amt += 0.02;
  }

  window_close(&window);
  free(buf);
  return 0;
}
