#include <stdio.h>
#include <time.h>

#include "../src/font.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

/* 
options:
  GFX_DEBUG
  GFX_IMPLEMENT
  GFX_USE_MALLOC
*/

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#include "../src/geometry.h"

float cube_colors[] = {
  1.0, 0.0, 0.0,
  1.0, 0.0, 0.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  0.0, 1.0, 0.0,
  0.0, 1.0, 0.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0
};

void draw_cube (float rotate_amt, float x, float y, float z)
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity(); /* step 0: init matrix */

  /* matrices must be multiplied in reverse order of the steps */
  gfx_translate(x, y, z); /* step 5: translate */
  gfx_rotate(1, 0, 0, rotate_amt); /* step 4: rotate about x axis */
  gfx_rotate(0, 1, 0, rotate_amt); /* step 3: rotate about y axis */
  gfx_scale(10, 10, 10); /* step 2: make larger */
  gfx_translate(-0.5, -0.5, -0.5); /* step 1: move to center */

  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12, cube_colors, 12);
  gfx_draw_arrays(0, -1);
}

int random_number (int range)
{
  int half = range / 2;
  return (rand() % range) - half;
}

#define NUM_CUBES 1000

float cube_x_pos[NUM_CUBES];
float cube_y_pos[NUM_CUBES];
float cube_z_pos[NUM_CUBES];

int main (int argc, char **argv)
{
  int width = 960;
  int height = 540;
  unsigned int* buf;
  float *zbuf;
  float fov = 70;
  window_t window;
  int is_paused = 0;
  int mode = 0;
  int frame, start;
  float rotate_amt = 0;
  char debug_string[50];
  int i;
  float cam_speed = 20;

  srand(time(NULL));

  for (i = 0; i < NUM_CUBES; i++) {
    cube_x_pos[i] = random_number(NUM_CUBES / 2);
    cube_z_pos[i] = random_number(NUM_CUBES / 2);
    cube_y_pos[i] = random_number(100);
  }

  buf = (unsigned int *)malloc((width * height) * sizeof(unsigned int));
  zbuf = (float *)malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  window_open(&window, "GFX", width, height);

  gfx_init();
  gfx_bind_render_target(buf, width, height);
  gfx_bind_depth_buffer(zbuf);

  while (!window.quit) {
    gfx_matrix_mode(GFX_PROJECTION_MATRIX);
    gfx_perspective(fov, (float)width / (float)height, 1, 1000.0);

    gfx_matrix_mode(GFX_VIEW_MATRIX);

    start = SDL_GetTicks();

    if (window.keys.a) gfx_translate(cam_speed, 0, 0);
    if (window.keys.d) gfx_translate(-cam_speed, 0, 0);
    if (window.keys.w) gfx_translate(0, 0, -cam_speed);
    if (window.keys.s) gfx_translate(0, 0, cam_speed);

    if (window.keys.left) fov -= 0.5;
    if (window.keys.right) fov += 0.5;

    if (window.keys.up) gfx_translate(0, -cam_speed, 0);
    if (window.keys.down) gfx_translate(0, cam_speed, 0);

    if (window.keys.p) is_paused = is_paused == 0 ? 1 : 0;

    if (window.keys._1) mode = GFX_FLAT_FILL_MODE;
    if (window.keys._2) mode = GFX_WIREFRAME_MODE;

    gfx_draw_mode(mode);

    for (i = 0; i < NUM_CUBES; i++) {
      draw_cube(rotate_amt, cube_x_pos[i], cube_y_pos[i], cube_z_pos[i]);
    }

    frame = SDL_GetTicks() - start;

    sprintf(debug_string, "frame: %dms", frame);

    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);

    window_update(&window, buf);

    gfx_clear();

    if (!is_paused) rotate_amt += 0.02;
  }

  window_close(&window);
  free(buf);
  return 0;
}