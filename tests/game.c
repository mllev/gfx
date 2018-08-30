#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

#define BMPREAD_IMPLEMENT
#include "../deps/bmpread.h"

#include "../src/geometry.h"

float cube_uvs[] = {
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0, 1.0, 0.0
};

void draw_frame (unsigned int *texture)
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
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_UV, cube_uvs);
  gfx_bind_texture(texture, 64, 64);
  gfx_draw_arrays(0, -1);

  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(0, -3, 25);
  gfx_scale(25, 3, 25);
  gfx_translate(-0.5, -0.5, -0.5);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_COLOR, NULL);
  gfx_draw_arrays(0, -1);
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
  float fov = 70;
  unsigned int frame, start;
  char debug_string[50];

  /* please figure out why relative paths don't work */
  unsigned int* texture = load_texture("/Users/matthewlevenstein/Desktop/projects/gfx/textures/wood.bmp");

  buf = (unsigned int *)malloc((width * height) * sizeof(unsigned int));
  zbuf = (float *)malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  window_open(&window, "Game", width, height);

  gfx_init();
  gfx_bind_render_target(buf, width, height);
  gfx_bind_depth_buffer(zbuf);
  gfx_set_projection(fov, (float)width / (float)height, 1);

  while (!window.quit) {
    start = SDL_GetTicks();

    draw_frame(texture);

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
