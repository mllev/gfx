#include <stdio.h>

#include "../src/font.h"

#define GFX_IMPLEMENT
#include "../src/gfx.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

#define BMPREAD_IMPLEMENT
#include "../deps/bmpread.h"

#include "../src/geometry.h"

#define DIR_FORWARDS 1
#define DIR_BACKWARDS 2
#define DIR_LEFT 3
#define DIR_RIGHT 4

typedef struct {
  float x, y, z;
  float speed;
  int move_direction;
  float move_distance;
  float r, g, b;
  float width, height, depth;
  int has_exited;
} entity;

struct {
  int has_begun;
  int current_level;
  int num_entities;

  struct {
    float x, y, z;
    float speed;
  } camera;

  entity player;
  entity entities[100];
  struct {
    float start, end, val;
  } colliders[100];

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
  gfx_scale(3, 3, 3);
  gfx_draw_arrays(0, -1);

  gfx_identity();
  gfx_translate(x_pos + 3, 1, STATE.board.depth);
  gfx_scale(3, 3, 3);
  gfx_draw_arrays(0, -1);

  gfx_identity();
  gfx_translate(x_pos - 3, 4, STATE.board.depth);
  gfx_scale(9, 3, 3);
  gfx_draw_arrays(0, -1);
}

#define set_text(buf) { \
  sprintf(STATE.text_buffer, buf); \
  STATE.text_width = strlen(buf) * 8; \
}

void update_entity (entity *e)
{
  switch (e->move_direction) {
    case DIR_FORWARDS:
      if (e->speed < e->move_distance) {
        e->z += e->speed;
        e->move_distance -= e->speed;
      } else {
        e->z += e->move_distance;
        e->move_distance = e->move_direction = 0;
      }
      break;
    case DIR_BACKWARDS:
      if (e->speed < e->move_distance) {
        e->z -= e->speed;
        e->move_distance -= e->speed;
      } else {
        e->z -= e->move_distance;
        e->move_distance = e->move_direction = 0;
      }
      break;
    case DIR_RIGHT:
      if (e->speed < e->move_distance) {
        e->x += e->speed;
        e->move_distance -= e->speed;
      } else {
        e->x += e->move_distance;
        e->move_distance = e->move_direction = 0;
      }
      break;
    case DIR_LEFT:
      if (e->speed < e->move_distance) {
        e->x -= e->speed;
        e->move_distance -= e->speed;
      } else {
        e->x -= e->move_distance;
        e->move_distance = e->move_direction = 0;
      }
      break;
    default: break;
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
  int i, max;

  for (i = 0, max = STATE.num_entities; i < max; i++) {
    gfx_matrix_mode(GFX_MODEL_MATRIX);
    gfx_identity();
    gfx_translate(STATE.entities[i].x, STATE.entities[i].y, STATE.entities[i].z);
    gfx_scale(STATE.entities[i].width, STATE.entities[i].height, STATE.entities[i].depth);
    gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
    gfx_bind_attr(GFX_ATTR_RGB, brown_color);
    gfx_draw_arrays(0, -1);
  }
}

void add_entity (float x, float y, float z, float w, float h, float d)
{
  int i = STATE.num_entities++;

  STATE.entities[i].x = x;
  STATE.entities[i].y = y;
  STATE.entities[i].z = z;
  STATE.entities[i].width = w;
  STATE.entities[i].height = h;
  STATE.entities[i].depth = d;
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

    draw_board();
    draw_exit(STATE.exit_x, white_color);
    draw_player();
    draw_entities();

    if (STATE.current_level > 0) {
      draw_exit(12, brown_color);
      draw_exit(30, brown_color);
    }
  }
}

int in_range (float v1, float v2, float p)
{
  return (
    (p < v2 && (fabs(v2 - p) > 0.0001)) && 
    (p > v1 || (fabs(v1 - p) < 0.0001))
  );
}

float get_nearest_collider (int direction)
{
  int i;
  float nearest;
  float nearest_z;
  float nearest_x;
  float px = STATE.player.x;
  float pz = STATE.player.z;

  switch (direction) {
    case DIR_FORWARDS: {
      nearest_z = 1000;
      for (i = 0; i < STATE.num_entities; i++) {
        float ex1 = STATE.entities[i].x;
        float ex2 = STATE.entities[i].x + STATE.entities[i].width;

        if (in_range(ex1, ex2, px)) {
          float ez = STATE.entities[i].z;
          if (ez > STATE.player.z) {
            if (ez < nearest_z) {
              nearest_z = ez;
            }
          }
        }
      }
      nearest = nearest_z;
    } break;
    case DIR_BACKWARDS: {
      nearest_z = -1000;
      for (i = 0; i < STATE.num_entities; i++) {
        float ex1 = STATE.entities[i].x;
        float ex2 = STATE.entities[i].x + STATE.entities[i].width;

        if (in_range(ex1, ex2, px)) {
          float ez = STATE.entities[i].z;
          if (ez < STATE.player.z) {
            if (ez > nearest_z) {
              nearest_z = ez + STATE.entities[i].depth;
            }
          }
        }
      }
      nearest = nearest_z;
    } break;
    case DIR_RIGHT: {
      nearest_x = 1000;
      for (i = 0; i < STATE.num_entities; i++) {
        float ez1 = STATE.entities[i].z;
        float ez2 = STATE.entities[i].z + STATE.entities[i].depth;

        if (in_range(ez1, ez2, pz)) {
          float ex = STATE.entities[i].x;
          if (ex > STATE.player.x) {
            if (ex < nearest_x) {
              nearest_x = ex;
            }
          }
        }
      }
      nearest = nearest_x;
    } break;
    case DIR_LEFT: {
      nearest_x = -1000;
      for (i = 0; i < STATE.num_entities; i++) {
        float ez1 = STATE.entities[i].z;
        float ez2 = STATE.entities[i].z + STATE.entities[i].depth;

        if (in_range(ez1, ez2, pz)) {
          float ex = STATE.entities[i].x;
          if (ex < STATE.player.x) {
            if (ex > nearest_x) {
              nearest_x = ex + STATE.entities[i].width;
            }
          }
        }
      }
      nearest = nearest_x;
    } break;
    default:
      nearest = 1000;
      break;
  }

  return nearest;
}

void move_player (int direction)
{
  float top = STATE.board.z + STATE.board.depth;
  float bottom = STATE.board.z;
  float right = STATE.board.x + STATE.board.width;
  float left = STATE.board.x;

  float player_top = STATE.player.z + STATE.player.depth;
  float player_bottom = STATE.player.z;
  float player_right = STATE.player.x + STATE.player.width;
  float player_left = STATE.player.x;

  float speed = STATE.player.speed;
  float depth = STATE.player.depth;
  float width = STATE.player.width;

  float nearest = get_nearest_collider(direction);

  STATE.player.move_direction = direction;

  /* @todo: collision with entities and exits */

  switch (direction) {
    case DIR_FORWARDS: {
      if (nearest < top) top = nearest;
      STATE.player.move_distance = player_top + speed < top ? depth : 0;
    } break;
    case DIR_BACKWARDS: {
      if (nearest > bottom) bottom = nearest;
      STATE.player.move_distance = player_bottom - speed > bottom ? depth : 0;
    } break;
    case DIR_LEFT: {
      if (nearest > left) left = nearest;
      STATE.player.move_distance = player_left - speed > left ? width : 0;
    } break;
    case DIR_RIGHT: {
      if (nearest < right) right = nearest;
      STATE.player.move_distance = player_right + speed < right ? width : 0;
    } break;
  }
}

void reset_player ()
{
  STATE.player.x = 0;
  STATE.player.z = 0;
  STATE.player.has_exited = 0;
}

void update_game ()
{
  switch (STATE.current_level) {
    case 0: {
      STATE.board.x = 0;
      STATE.board.z = 0;
      STATE.board.width = 15;
      STATE.board.height = 1;
      STATE.board.depth = 15;
      STATE.exit_x = 6;
      STATE.camera.x = 7.5;
      STATE.num_entities = 0;
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
      STATE.num_entities = 0;

      add_entity(0, 1, 9, 6, 3, 3);
      add_entity(0, 1, 6, 3, 3, 3);
      add_entity(12, 1, 3, 6, 3, 3);
    } break;
  }

  update_entity(&STATE.player);
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

    if (!STATE.player.move_direction) {
      if (window.keys.up) {
        move_player(DIR_FORWARDS);
      } else if (window.keys.down) {
        move_player(DIR_BACKWARDS);
      } else if (window.keys.left) {
        move_player(DIR_LEFT);
      } else if (window.keys.right) {
        move_player(DIR_RIGHT);
      }
    }

    if (window.keys._1) {
      STATE.current_level = 0;
    }

    if (window.keys._2) {
      STATE.current_level = 1;
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
