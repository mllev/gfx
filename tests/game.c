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
  float move_distance;
  float r, g, b;
  float width, height, depth;
  struct { float x, z; } direction;
  int is_exiting;
  int is_moving;
  int index;
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
    float width, height, depth;
    float x, y, z;
    float r, g, b;
  } board;

  float exit_x;

  char text_buffer[100];
  int text_width;

  int window_width, window_height;
} STATE;

float blue_color[]  = { 0.349, 0.647, 0.847 };
float white_color[] = {   1.0,   1.0,   1.0 };
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

void draw_entity (entity *e, float *color)
{
  gfx_matrix_mode(GFX_MODEL_MATRIX);
  gfx_identity();
  gfx_translate(e->x, e->y, e->z);
  gfx_scale(e->width, e->height, e->depth);
  gfx_bind_arrays(cube_vertices, 8, cube_indices, 12);
  gfx_bind_attr(GFX_ATTR_RGB, color);
  gfx_draw_arrays(0, -1);
}

#define set_text(buf) { \
  sprintf(STATE.text_buffer, buf); \
  STATE.text_width = strlen(buf) * 8; \
}

int entity_has_collided_with_entity (entity *e1, entity *e2)
{
  float e1left = e1->x, e1right = e1->x + e1->width;
  float e1bottom = e1->z, e1top = e1->z + e1->depth;

  float e2left = e2->x, e2right = e2->x + e2->width;
  float e2bottom = e2->z, e2top = e2->z + e2->depth;

  if (e1right > (e2left + 0.0001) && e2right > (e1left + 0.0001)) {
    if (e1top > (e2bottom + 0.0001) && e2top > (e1bottom + 0.0001)) {
      return 1;
    }
  }

  return 0;
}

int entity_has_collided_with_walls (entity *e)
{
  float left = e->x, right = e->x + e->width;
  float bottom = e->z, top = e->z + e->depth;

  if (fabs(e->x - STATE.exit_x) < 0.0001 && top > STATE.board.z + STATE.board.depth) {
    return 0;
  }

  if (
    right > STATE.board.x + STATE.board.width + 0.0001 || 
    left < STATE.board.x - 0.0001 ||
    top > STATE.board.z + STATE.board.depth + 0.0001 ||
    bottom < STATE.board.z - 0.0001
  ) return 1;

  return 0;
}

void update_entity (entity *e)
{
  if (e->is_moving) {
    float speed = e->speed;
    float xstep, zstep;

    if (speed > e->move_distance) {
      speed = e->move_distance;
      e->move_distance = e->is_moving = e->is_exiting = 0;
    } else {
      e->move_distance -= speed;
    }

    xstep = e->direction.x * speed;
    zstep = e->direction.z * speed;

    e->x += xstep;
    e->z += zstep;

    /* @todo: get remaining distance to the walls/entities if there's a collision */

    if (entity_has_collided_with_walls(e)) {
      e->x -= xstep;
      e->z -= zstep;
      e->move_distance = e->is_moving = 0;
    } else {
      int i;
      for (i = 0; i < STATE.num_entities; i++) {
        if (i != e->index && entity_has_collided_with_entity(e, &STATE.entities[i])) {
          e->x -= xstep;
          e->z -= zstep;
          break;
        }
      }
    }
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
  STATE.entities[i].index = i;
}

void draw_frame ()
{
  int i;

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
    draw_entity(&STATE.player, white_color);
    draw_exit(STATE.exit_x, white_color);

    for (i = 0; i < STATE.num_entities; i++) {
      draw_entity(&STATE.entities[i], brown_color);
    }
  }
}

void move_entity (entity *e, int direction)
{
  switch (direction) {
    case DIR_FORWARDS: {
      e->direction.x = 0;
      e->direction.z = 1;
      e->move_distance = e->depth;
    } break;
    case DIR_BACKWARDS: {
      e->direction.x = 0;
      e->direction.z = -1;
      e->move_distance = e->depth;
    } break;
    case DIR_LEFT: {
      e->direction.z = 0;
      e->direction.x = -1;
      e->move_distance = e->width;
    } break;
    case DIR_RIGHT: {
      e->direction.z = 0;
      e->direction.x = 1;
      e->move_distance = e->width;
    } break;
  }

  STATE.player.is_moving = 1;
}

void init_level_1 ()
{
  STATE.board.x = 0;
  STATE.board.z = 0;
  STATE.board.width = 15;
  STATE.board.height = 1;
  STATE.board.depth = 15;

  STATE.exit_x = 6;

  STATE.camera.y = 30;
  STATE.camera.x = 7.5;
  STATE.camera.z = -7;
  STATE.camera.speed = 0.1;

  STATE.player.x = 0;
  STATE.player.z = 0;
  STATE.player.y = 1;
  STATE.player.speed = 0.65;
  STATE.player.width = 3;
  STATE.player.height = 3;
  STATE.player.depth = 3;
  STATE.player.is_exiting = 0;

  STATE.num_entities = 0;
}

void init_level_2 ()
{
  STATE.board.x = 0;
  STATE.board.z = 0;
  STATE.board.width = 46.5;
  STATE.board.height = 1;
  STATE.board.depth = 15;

  STATE.exit_x = 7.5;

  STATE.camera.y = 30;
  STATE.camera.x = 22.5;
  STATE.camera.z = -7;
  STATE.camera.speed = 0.1;

  STATE.player.x = 0;
  STATE.player.z = 0;
  STATE.player.y = 1;
  STATE.player.speed = 0.65;
  STATE.player.width = 3;
  STATE.player.height = 3;
  STATE.player.depth = 3;
  STATE.player.is_exiting = 0;

  STATE.num_entities = 0;
}

void init_level_3 ()
{
  STATE.board.x = 0;
  STATE.board.z = 0;
  STATE.board.width = 45;
  STATE.board.height = 1;
  STATE.board.depth = 15;

  STATE.exit_x = 21;

  STATE.camera.y = 30;
  STATE.camera.x = 22.5;
  STATE.camera.z = -7;
  STATE.camera.speed = 0.1;

  STATE.player.x = 0;
  STATE.player.z = 0;
  STATE.player.y = 1;
  STATE.player.speed = 0.65;
  STATE.player.width = 3;
  STATE.player.height = 3;
  STATE.player.depth = 3;
  STATE.player.is_exiting = 0;

  STATE.num_entities = 0;

  add_entity(0, 1, 9, 6, 3, 3);
  add_entity(0, 1, 6, 3, 3, 3);
  add_entity(12, 1, 3, 6, 3, 3);
}

void update_game ()
{
  update_entity(&STATE.player);
}

void init_game ()
{
  memset(&STATE, 0, sizeof(STATE));
  STATE.player.index = -1;
  init_level_1();
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

    if (!STATE.player.is_moving) {
      if (window.keys.up) {
        move_entity(&STATE.player, DIR_FORWARDS);
      } else if (window.keys.down) {
        move_entity(&STATE.player, DIR_BACKWARDS);
      } else if (window.keys.left) {
        move_entity(&STATE.player, DIR_LEFT);
      } else if (window.keys.right) {
        move_entity(&STATE.player, DIR_RIGHT);
      }
    }

    if (window.keys._1) {
      STATE.current_level = 0;
      init_level_1();
    }

    if (window.keys._2) {
      STATE.current_level = 1;
      init_level_2();
    }

    if (window.keys._3) {
      STATE.current_level = 2;
      init_level_3();
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
