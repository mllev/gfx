#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

int main (void) {
  window_t window;
  int width = 640, height = 360;
  unsigned int* framebuffer;
  int bufsize = width * height * sizeof(unsigned int);

  if (!(framebuffer = malloc(bufsize))) {
    return 1;
  }

  window_open(&window, "Game?", 640, 360);

  gfx_init();
  gfx_bind_render_target(framebuffer, width, height);

  while (!window.quit) {
    gfx_draw_text_8x8(ascii, "Welcome to game.", 16, width / 2 - 64, height / 2 - 4);
    window_update(&window, framebuffer);
  }

  window_close(&window);

  return 0;
}
