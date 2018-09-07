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
  int has_begun;
  int current_level;

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
    float r, g, b;
    float width, height, depth;
    float current_x, current_z;
    int has_exited;
  } player;

  struct {
    float width, height, depth;
    float x, y, z;
    float r, g, b;
  } board;

  float exit_x;

  char text_buffer[100];
  int text_width;

  int window_width, window_height;
} STATE;

float blue_color[]  = { 0.349, 0.647, 0.847, };
float white_color[] = { 1.0, 1.0, 1.0 };
float brown_color[] = { 0.573, 0.416, 0.176 };

void draw_board ()
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_scale(STATE.board.width, STATE.board.height, STATE.board.depth);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, blue_color);
  gfx_draw_arrays(0, -1);
}

void draw_exit (float x_pos, float *color)
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, color);

  gfx_identity();
  gfx_translate(x_pos - 3, 1, STATE.board.depth);
  gfx_scale(3, 6, 3);
  gfx_draw_arrays(0, -1);

  gfx_identity();
  gfx_translate(x_pos + 3, 1, STATE.board.depth);
  gfx_scale(3, 6, 3);
  gfx_draw_arrays(0, -1);

  gfx_identity();
  gfx_translate(x_pos, 4, STATE.board.depth);
  gfx_scale(3, 3, 3);
  gfx_draw_arrays(0, -1);
}

#define set_text(buf) { \
  sprintf(STATE.text_buffer, buf); \
  STATE.text_width = strlen(buf) * 8; \
}

void update_player ()
{
  float max_board_x = STATE.board.width - STATE.player.width;
  float max_board_z = STATE.board.depth - STATE.player.depth;

  float max_tile_x = STATE.player.current_x + STATE.player.width;
  float max_tile_z = STATE.player.current_z + STATE.player.depth;

  float min_tile_x = STATE.player.current_x - STATE.player.width;
  float min_tile_z = STATE.player.current_z - STATE.player.depth;

  if (STATE.player.is_moving_forwards) {
    STATE.player.z += STATE.player.speed;

    if (STATE.player.z > max_board_z && STATE.player.x != STATE.exit_x) {
      STATE.player.z = max_board_z;
      STATE.player.is_moving_forwards = 0;
      STATE.player.current_z = STATE.player.z;
    } else if (STATE.player.z > max_tile_z) {
      STATE.player.z = max_tile_z;
      STATE.player.is_moving_forwards = 0;
      STATE.player.current_z = STATE.player.z;

      if (max_tile_z > max_board_z) {
        STATE.player.has_exited = 1;
      }
    }
  }

  if (STATE.player.is_moving_backwards) {
    float min = min_tile_z > 0 ? min_tile_z : 0;

    STATE.player.z -= STATE.player.speed;

    if (STATE.player.z < min) {
      STATE.player.z = min;
      STATE.player.is_moving_backwards = 0;
      STATE.player.current_z = STATE.player.z;
    }
  }

  if (STATE.player.is_moving_right) {
    float max = max_tile_x < max_board_x ? max_tile_x : max_board_x;

    STATE.player.x += STATE.player.speed;

    if (STATE.player.x > max && (STATE.player.x != 12)) {
      STATE.player.x = max;
      STATE.player.is_moving_right = 0;
      STATE.player.current_x = STATE.player.x;
    }
  }

  if (STATE.player.is_moving_left) {
    float min = min_tile_x > 0 ? min_tile_x : 0;

    STATE.player.x -= STATE.player.speed;

    if (STATE.player.x < min) {
      STATE.player.x = min;
      STATE.player.is_moving_left = 0;
      STATE.player.current_x = STATE.player.x;
    }
  }
}

void draw_player ()
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(STATE.player.x, STATE.player.y, STATE.player.z);
  gfx_scale(STATE.player.width, STATE.player.height, STATE.player.depth);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, white_color);
  gfx_draw_arrays(0, -1);
}

void draw_entities ()
{
  return;
}

void draw_frame ()
{
  if (!STATE.has_begun) {
    int text_x = STATE.window_width / 2 - (STATE.text_width / 2);
    int text_y = STATE.window_height / 2 - 4;

    set_text("Press ENTER to begin.");
    gfx_draw_text_8x8(ascii, STATE.text_buffer, strlen(STATE.text_buffer), text_x, text_y);
  } else {
    gfx_matrix_mode(GFX_VIEW_MATRIX);
    gfx_identity();
    gfx_rotate(1, 0, 0, -1.01);
    gfx_translate(-STATE.camera.x, -STATE.camera.y, -STATE.camera.z);

    draw_player();
    draw_board();
    draw_entities();
    draw_exit(STATE.exit_x, white_color);

    if (STATE.current_level > 0) {
      draw_exit(12, brown_color);
      draw_exit(30, brown_color);
    }
  }
}

void reset_player ()
{
  STATE.player.x = 0;
  STATE.player.z = 0;
  STATE.player.current_x = 0;
  STATE.player.current_z = 0;
  STATE.player.is_moving_forwards = 0;
  STATE.player.is_moving_backwards = 0;
  STATE.player.is_moving_right = 0;
  STATE.player.is_moving_left = 0;
  STATE.player.has_exited = 0;
}

void update_game ()
{
  if (STATE.player.has_exited) {
    reset_player();
    STATE.current_level++;
  }

  switch (STATE.current_level) {
    case 0: {
      STATE.board.x = 0;
      STATE.board.z = 0;
      STATE.board.width = 15;
      STATE.board.height = 1;
      STATE.board.depth = 15;
      STATE.exit_x = 6;
      STATE.camera.x = 7.5;
    } break;
    case 1:
    default: {
      STATE.board.x = 0;
      STATE.board.z = 0;
      STATE.board.width = 45;
      STATE.board.height = 1;
      STATE.board.depth = 15;
      STATE.exit_x = 21;
      STATE.camera.x = 22.5;
    } break;
  }

  update_player();
}

void init_game ()
{
  memset(&STATE, 0, sizeof(STATE));

  STATE.camera.y = 30;
  STATE.camera.x = 7.5;
  STATE.camera.z = -7;
  STATE.camera.speed = 0.1;

  STATE.player.y = 1;
  STATE.player.speed = 0.65;
  STATE.player.width = 3;
  STATE.player.height = 3;
  STATE.player.depth = 3;
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
  int width = 960, height = 540;
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

  STATE.window_width = width;
  STATE.window_height = height;

  while (!window.quit) {
    start = SDL_GetTicks();

    if (window.keys.w) {
      STATE.camera.z += STATE.camera.speed;
    }

    if (window.keys.s) {
      STATE.camera.z -= STATE.camera.speed;
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

    if (window.keys.enter && !STATE.has_begun) {
      STATE.has_begun = 1;
    }

    update_game();
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
