#include <stdio.h>
#include <time.h>

#include "../src/font.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

#define BMPREAD_IMPLEMENT
#include "../deps/bmpread.h"

/* 
options:
  GFX_DEBUG
  GFX_IMPLEMENT
  GFX_USE_MALLOC
*/

#define GFX_IMPLEMENT
#include "../src/gfx.h"

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

float cube_uvs[] = {
  0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 1.0,
  1.0, 1.0, 1.0, 0.0, 0.0, 0.0
};

void draw_cube (float rotate_amt, float x, float y, float z, unsigned int *texture)
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity(); /* step 0: init matrix */

  /* matrices must be multiplied in reverse order of the steps */
  gfx_translate(x, y, z); /* step 5: translate */
  gfx_rotate(1, 0, 0, rotate_amt); /* step 4: rotate about x axis */
  gfx_rotate(0, 1, 0, rotate_amt); /* step 3: rotate about y axis */
  gfx_scale(10, 10, 10); /* step 2: make larger */
  gfx_translate(-0.5, -0.5, -0.5); /* step 1: move to center */

  gfx_bind_primitive(GFX_PRIMITIVE_CUBE);
  gfx_bind_attr(GFX_ATTR_UVS, cube_uvs);
  gfx_bind_texture(texture, 64, 64);
  /* gfx_bind_attr(GFX_ATTR_COLORS, cube_colors); */
  gfx_draw_arrays(0, -1);
}

int random_number (int range)
{
  int half = range / 2;
  return (rand() % range) - half;
}

#define NUM_CUBES 10000

float cube_x_pos[NUM_CUBES];
float cube_y_pos[NUM_CUBES];
float cube_z_pos[NUM_CUBES];

unsigned int* load_texture (const char *file)
{ 
  int x, y, w = 64, h = 64;
  bmpread_t bmp1;
  unsigned int *texture;
  if (bmpread(file, 0, &bmp1)) {
    texture = (unsigned int *)malloc(sizeof(unsigned int) * bmp1.width * bmp1.height);
    for (x = 0; x < w; x++) {
      for (y = 0; y < h; y++) {
        int idx = w * y + x;
        int r = bmp1.rgb_data[idx*3];
        int g = bmp1.rgb_data[idx*3+1];
        int b = bmp1.rgb_data[idx*3+2];

        texture[idx] = ((unsigned int)255 << 24) | (r << 16) | (g << 8) | b;
      }
    }
    return texture;
  } else {
    printf("No file.\n");
    return NULL;
  }
}

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

  unsigned int* texture = load_texture("./textures/wood.bmp");

  srand(time(NULL));

  for (i = 0; i < NUM_CUBES; i++) {
    cube_x_pos[i] = random_number(NUM_CUBES / 4);
    cube_z_pos[i] = random_number(NUM_CUBES / 4);
    cube_y_pos[i] = random_number(100);
  }

  buf = (unsigned int *)malloc((width * height) * sizeof(unsigned int));
  zbuf = (float *)malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  memset(buf, 0, (width * height) * sizeof(unsigned int));
  memset(zbuf, 0, (width * height) * sizeof(float));

  window_open(&window, "GFX", width, height);

  gfx_init();
  gfx_bind_render_target(buf, width, height);
  gfx_bind_depth_buffer(zbuf);

  gfx_matrix_mode(GFX_VIEW_MATRIX);
  gfx_identity();

  while (!window.quit) {
    gfx_set_projection(fov, (float)width / (float)height, 1);

    gfx_matrix_mode(GFX_VIEW_MATRIX);

    start = SDL_GetTicks();

    window_poll(&window);

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
      draw_cube(rotate_amt, cube_x_pos[i], cube_y_pos[i], cube_z_pos[i], texture);
    }

    frame = SDL_GetTicks() - start;

    sprintf(debug_string, "fps: %f", 1000.0 / (float)frame);

    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);

    window_render(&window, buf);

    gfx_clear();

    if (!is_paused) rotate_amt += 0.02;
  }

  window_close(&window);
  free(buf);
  return 0;
}
