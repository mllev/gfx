#include <stdio.h>
#include <time.h>

#include <iostream>

#include "../src/font.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../deps/tiny_obj_loader.h"

#define WINDOW_IMPLEMENT
#include "../src/window.h"

/* 
options:
  GFX_DEBUG
  GFX_IMPLEMENT
  GFX_USE_MALLOC
*/

#define GFX_MAX_VERTICES 1000000
#define GFX_MAX_FACES 1000000
#define GFX_USE_MALLOC
#define GFX_IMPLEMENT
#include "../src/gfx.h"

int mesh_num_vertices;
int mesh_num_indices;

float *mesh_colors;
float *mesh_vertices;
int *mesh_indices;

void load_obj (const char *file)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  bool ret = tinyobj::LoadObj(
    &attrib, &shapes, &materials, 
    &err,
    file,
    "./models/", 
    true
  );

  std::cout << err;

  int num_faces = 0;
  int index = 0;
  int color_idx = 0;

  for (size_t s = 0; s < shapes.size(); s++) {
    num_faces += shapes[s].mesh.num_face_vertices.size();
  }

  mesh_num_vertices = attrib.vertices.size();
  mesh_num_indices = num_faces;

  mesh_colors = (float *)malloc(sizeof(float) * 3 * num_faces);
  mesh_vertices = (float *)malloc(sizeof(float) * 3 * attrib.vertices.size());
  mesh_indices = (int *)malloc(sizeof(unsigned int) * 3 * num_faces);

  for (int i = 0; i < attrib.vertices.size(); i++)
    mesh_vertices[i] = attrib.vertices[i];

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;

    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      int mid = shapes[s].mesh.material_ids[f];

      mesh_indices[index++] = shapes[s].mesh.indices[index_offset + 0].vertex_index;
      mesh_indices[index++] = shapes[s].mesh.indices[index_offset + 1].vertex_index;
      mesh_indices[index++] = shapes[s].mesh.indices[index_offset + 2].vertex_index;

      if (mid > -1) {
        mesh_colors[color_idx++] = materials[mid].diffuse[0];
        mesh_colors[color_idx++] = materials[mid].diffuse[1];
        mesh_colors[color_idx++] = materials[mid].diffuse[2];
      } else {
        mesh_colors[color_idx++] = 1.0;
        mesh_colors[color_idx++] = 1.0;
        mesh_colors[color_idx++] = 1.0;
      }

      index_offset += 3;

      // per-face material
      shapes[s].mesh.material_ids[f];
    }
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
  char debug_string[50];
  int i;
  float cam_speed = 20;

  buf = (unsigned int *)malloc((width * height) * sizeof(unsigned int));
  zbuf = (float *)malloc((width * height) * sizeof(float));

  if (!buf || !zbuf) return 1;

  window_open(&window, "GFX", width, height);

  load_obj("./models/mountain.obj");

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

    if (window.keys._1) mode = GFX_FLAT_FILL_MODE;
    if (window.keys._2) mode = GFX_WIREFRAME_MODE;

    gfx_draw_mode(mode);

    gfx_matrix_mode(GFX_MODEL_MATRIX);
    gfx_identity();
    gfx_rotate(0, 1, 0, -(GFX_PI / 2));
    gfx_scale(100, 200, 100);
    gfx_bind_arrays(mesh_vertices, mesh_num_vertices, mesh_indices, mesh_num_indices, mesh_colors, mesh_num_indices);
    gfx_draw_arrays(0, -1);

    frame = SDL_GetTicks() - start;

    sprintf(debug_string, "frame: %dms", frame);

    gfx_draw_text_8x8(ascii, debug_string, strlen(debug_string), 0, 0);

    window_update(&window, buf);

    gfx_clear();
  }

  window_close(&window);
  free(buf);
  return 0;
}
