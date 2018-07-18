#ifndef _GFX_H
#define _GFX_H

#include <math.h> /* sinf cosf */

typedef unsigned int u32;

#define PI 3.141592653589793

#ifndef GFX_MAX_VERTICES
#define GFX_MAX_VERTICES 10000
#endif

#ifndef GFX_MAX_FACES
#define GFX_MAX_FACES 10000
#endif

#define GFX_MODEL_MATRIX 1
#define GFX_PROJECTION_MATRIX 2
#define GFX_VIEW_MATRIX 3

#define GFX_FRACBITS 16
#define gfx_fixed16(x) ((u32)(x * 65535))

typedef struct _gfxv2 gfxv2;
typedef struct _gfxv3 gfxv3;
typedef struct _gfxv4 gfxv4;
typedef struct _gfxm4 gfxm4;

typedef struct _gfxvert gfxvert;

struct _gfxv2 { float x, y; };
struct _gfxv3 { float x, y, z; };
struct _gfxv4 { float x, y, z, w; };

struct _gfxm4 {
  float _00, _01, _02, _03;
  float _10, _11, _12, _13;
  float _20, _21, _22, _23;
  float _30, _31, _32, _33;
};

struct _gfxvert {
  gfxv4 camera_space;
  gfxv2 screen_space;
};

struct _gfx {
  u32* target;
  float *depth_buffer;

  int target_width;
  int target_height;

  float *vertices;
  int *indices;
  float *colors;

  int vertex_count;
  int index_count;
  int color_count;

  gfxm4 model;
  gfxm4 view;
  gfxm4 projection;
  gfxm4 transform;

  gfxm4* active;

  gfxvert vertex_pipe[GFX_MAX_VERTICES];
  gfxv4 face_pipe[GFX_MAX_FACES];
};

int gfx_init(void);
unsigned int gfx_memory_requirements(void);
void gfx_clear(void);

void gfx_bind_render_target(u32*, int, int);
void gfx_bind_depth_buffer(float*);
void gfx_bind_arrays(float*, int, int*, int, float*, int);

void gfx_draw_arrays(int, int);
void gfx_draw_text_8x8(char[][8], const char *, int, int, int);
void gfx_draw_line(float, float, float, float, u32);

void gfx_matrix_mode(int);
void gfx_rotate(float, float, float, float);
void gfx_translate(float, float, float);
void gfx_scale(float);
void gfx_identity(void);
void gfx_perspective(float, float, float, float);

#ifdef GFX_IMPLEMENT

/* all globals */
#ifdef GFX_USE_MALLOC

#include <stdlib.h>

static struct _gfx *__GFX;

#define GFX (*__GFX)

#else

static struct _gfx GFX;

#endif

#ifdef GFX_DEBUG

static void gfx_m4_print (gfxm4 *m)
{
  printf("%f %f %f %f\n", m->_00, m->_01, m->_02, m->_03);
  printf("%f %f %f %f\n", m->_10, m->_11, m->_12, m->_13);
  printf("%f %f %f %f\n", m->_20, m->_21, m->_22, m->_23);
  printf("%f %f %f %f\n", m->_30, m->_31, m->_32, m->_33);
  printf("\n");
}

static void gfx_v4_print (gfxv4 *v)
{
  printf("%f %f %f %f\n\n", v->x, v->y, v->z, v->w);
}

#endif

static void gfx_m4_mult (gfxm4 *r, gfxm4 *m1, gfxm4 *m2)
{
  r->_00 = m1->_00 * m2->_00 + m1->_01 * m2->_10 + m1->_02 * m2->_20 + m1->_03 * m2->_30;
  r->_01 = m1->_00 * m2->_01 + m1->_01 * m2->_11 + m1->_02 * m2->_21 + m1->_03 * m2->_31;
  r->_02 = m1->_00 * m2->_02 + m1->_01 * m2->_12 + m1->_02 * m2->_22 + m1->_03 * m2->_32;
  r->_03 = m1->_00 * m2->_03 + m1->_01 * m2->_13 + m1->_02 * m2->_23 + m1->_03 * m2->_33;

  r->_10 = m1->_10 * m2->_00 + m1->_11 * m2->_10 + m1->_12 * m2->_20 + m1->_13 * m2->_30;
  r->_11 = m1->_10 * m2->_01 + m1->_11 * m2->_11 + m1->_12 * m2->_21 + m1->_13 * m2->_31;
  r->_12 = m1->_10 * m2->_02 + m1->_11 * m2->_12 + m1->_12 * m2->_22 + m1->_13 * m2->_32;
  r->_13 = m1->_10 * m2->_03 + m1->_11 * m2->_13 + m1->_12 * m2->_23 + m1->_13 * m2->_33;

  r->_20 = m1->_20 * m2->_00 + m1->_21 * m2->_10 + m1->_22 * m2->_20 + m1->_23 * m2->_30;
  r->_21 = m1->_20 * m2->_01 + m1->_21 * m2->_11 + m1->_22 * m2->_21 + m1->_23 * m2->_31;
  r->_22 = m1->_20 * m2->_02 + m1->_21 * m2->_12 + m1->_22 * m2->_22 + m1->_23 * m2->_32;
  r->_23 = m1->_20 * m2->_03 + m1->_21 * m2->_13 + m1->_22 * m2->_23 + m1->_23 * m2->_33;

  r->_30 = m1->_30 * m2->_00 + m1->_31 * m2->_10 + m1->_32 * m2->_20 + m1->_33 * m2->_30;
  r->_31 = m1->_30 * m2->_01 + m1->_31 * m2->_11 + m1->_32 * m2->_21 + m1->_33 * m2->_31;
  r->_32 = m1->_30 * m2->_02 + m1->_31 * m2->_12 + m1->_32 * m2->_22 + m1->_33 * m2->_32;
  r->_33 = m1->_30 * m2->_03 + m1->_31 * m2->_13 + m1->_32 * m2->_23 + m1->_33 * m2->_33;
}

static void gfx_v4_mult (gfxv4 *r, gfxv4 *v, gfxm4 *m)
{
  r->x = m->_00 * v->x + m->_01 * v->y + m->_02 * v->z + m->_03 * v->w;
  r->y = m->_10 * v->x + m->_11 * v->y + m->_12 * v->z + m->_13 * v->w;
  r->z = m->_20 * v->x + m->_21 * v->y + m->_22 * v->z + m->_23 * v->w;
  r->w = m->_30 * v->x + m->_31 * v->y + m->_32 * v->z + m->_33 * v->w;
}

static void gfx_v4_init (gfxv4 *v, float x, float y, float z, float w)
{
  v->x = x;
  v->y = y;
  v->z = z;
  v->w = w;
}

static void gfx_m4_ident (gfxm4 *m)
{
  m->_00 = 1; m->_01 = 0; m->_02 = 0; m->_03 = 0;
  m->_10 = 0; m->_11 = 1; m->_12 = 0; m->_13 = 0;
  m->_20 = 0; m->_21 = 0; m->_22 = 1; m->_23 = 0;
  m->_30 = 0; m->_31 = 0; m->_32 = 0; m->_33 = 1;
}

static void gfx_project_to_screen (gfxv2 *s, gfxv4 *v)
{
  float zinv = 1.0f / v->w;

  s->x = ((v->x * zinv) + 1.0f) * (GFX.target_width / 2);
  s->y = ((v->y * zinv) + 1.0f) * (GFX.target_height / 2);

  /* necessary if screen is inverted */
  s->y = GFX.target_height - 1 - s->y;
}

static void gfx_m4_perspective (gfxm4 *m, float fov, float aspect, float n, float f)
{
  float y = 1.0 / tanf(fov / 2);

  m->_00 = y / aspect;
  m->_11 = y;
  m->_22 = 1;
  m->_32 = 1;
  m->_33 = 0;
}

static void gfx_m4_rotation (gfxm4 *m, float x, float y, float z, float a)
{
  float s = sinf(a);
  float c = cosf(a);

  float c1 = 1 - c;

  float xy = x * y, xx = x * x, xz = x * z;
  float yy = y * y, yz = y * z, zz = z * z;
  float xs = x * s, ys = y * s, zs = z * s;

  m->_00 = xx * c1 + c;
  m->_01 = xy * c1 - zs;
  m->_02 = xz * c1 + ys;
  m->_03 = 0;

  m->_10 = xy * c1 + zs;
  m->_11 = yy * c1 + c;
  m->_12 = yz * c1 - xs;
  m->_13 = 0;

  m->_20 = xz * c1 - ys;
  m->_21 = yz * c1 + xs;
  m->_22 = zz * c1 + c;
  m->_23 = 0;

  m->_30 = 0;
  m->_31 = 0;
  m->_32 = 0;
  m->_33 = 1;
}

static void gfx_m4_scale (gfxm4 *m, float x, float y, float z)
{
  m->_00 = x;  m->_01 = 0;  m->_02 = 0;  m->_03 = 0;
  m->_10 = 0;  m->_11 = y;  m->_12 = 0;  m->_13 = 0;
  m->_20 = 0;  m->_21 = 0;  m->_22 = z;  m->_23 = 0;
  m->_30 = 0;  m->_31 = 0;  m->_32 = 0;  m->_33 = 1;
}

static void gfx_m4_translation (gfxm4 *m, float x, float y, float z)
{
  m->_00 = 1;  m->_01 = 0;  m->_02 = 0;  m->_03 = x;
  m->_10 = 0;  m->_11 = 1;  m->_12 = 0;  m->_13 = y;
  m->_20 = 0;  m->_21 = 0;  m->_22 = 1;  m->_23 = z;
  m->_30 = 0;  m->_31 = 0;  m->_32 = 0;  m->_33 = 1;
}

static float gfx_v4_length (gfxv4 *v)
{
  return sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

static void gfx_v4_normalize (gfxv4 *out, gfxv4 *in)
{
  float m = gfx_v4_length(in);
  float d = 1 / m;

  out->x = in->x * d;
  out->y = in->y * d;
  out->z = in->z * d;
}

static void gfx_v4_crossp (gfxv4 *out, gfxv4 *a, gfxv4 *b)
{
  out->x = (a->y * b->z) - (a->z * b->y);
  out->y = (a->z * b->x) - (a->x * b->z);
  out->z = (a->x * b->y) - (a->y * b->x);
}

static float gfx_v4_dotp (gfxv4 *v1, gfxv4 *v2)
{
  return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

static void gfx_v4_sub (gfxv4 *out, gfxv4 *v1, gfxv4 *v2)
{
  out->x = v1->x - v2->x;
  out->y = v1->y - v2->y;
  out->z = v1->z - v2->z;
}

static void gfx_m4_copy (gfxm4 *a, gfxm4 *b)
{
  a->_00 = b->_00; a->_01 = b->_01; a->_02 = b->_02; a->_03 = b->_03;
  a->_10 = b->_10; a->_11 = b->_11; a->_12 = b->_12; a->_13 = b->_13;
  a->_20 = b->_20; a->_21 = b->_21; a->_22 = b->_22; a->_23 = b->_23;
  a->_30 = b->_30; a->_31 = b->_31; a->_32 = b->_32; a->_33 = b->_33;
}

static void gfx_v4_copy (gfxv4 *a, gfxv4 *b)
{
  a->x = b->x;
  a->y = b->y;
  a->z = b->z;
  a->w = b->w;
}

void gfx_clear ()
{
  int i = 0, l = GFX.target_width * GFX.target_height;
  for (; i < l; i++) GFX.target[i] = 0;
  for (; i < l; i++) GFX.depth_buffer[i] = 0.0;
}

int gfx_init ()
{
#ifdef GFX_USE_MALLOC
  __GFX = malloc(sizeof(struct _gfx));
  if (!__GFX) return 0;
  memset(__GFX, 0, sizeof(struct _gfx));
#endif

  gfx_m4_ident(&GFX.model);
  gfx_m4_ident(&GFX.view);
  gfx_m4_ident(&GFX.projection);

  return 1;
}

unsigned int gfx_memory_requirements ()
{
  return (unsigned int)sizeof(struct _gfx);
}

void gfx_matrix_mode (int m)
{
  switch (m) {
    case GFX_MODEL_MATRIX:
      GFX.active = &GFX.model;
    break;
    case GFX_VIEW_MATRIX:
      GFX.active = &GFX.view;
    break;
    case GFX_PROJECTION_MATRIX:
      GFX.active = &GFX.projection;
    break;
  }
}

void gfx_bind_render_target (unsigned int *target, int width, int height)
{
  GFX.target = target;
  GFX.target_width = width;
  GFX.target_height = height;
}

void gfx_bind_depth_buffer (float *depth_buffer)
{
  GFX.depth_buffer = depth_buffer;
}

void gfx_bind_arrays (float *vertices, int vsize, int *indices, int isize, float *colors, int csize)
{
  GFX.vertices = vertices;
  GFX.indices = indices;
  GFX.colors = colors;

  GFX.vertex_count = vsize;
  GFX.index_count = isize;
  GFX.color_count = csize;
}

void gfx_rotate (float x, float y, float z, float a)
{
  gfxm4 rotation;
  gfx_m4_rotation(&rotation, x, y, z, a);
  gfx_m4_mult(&GFX.transform, GFX.active, &rotation);
  gfx_m4_copy(GFX.active, &GFX.transform);
}

void gfx_translate (float x, float y, float z)
{
  gfxm4 translation;
  gfx_m4_translation(&translation, x, y, z);
  gfx_m4_mult(&GFX.transform, GFX.active, &translation);
  gfx_m4_copy(GFX.active, &GFX.transform);
}

void gfx_scale (float f)
{
  gfxm4 scale;
  gfx_m4_scale(&scale, f, f, f);
  gfx_m4_mult(&GFX.transform, GFX.active, &scale);
  gfx_m4_copy(GFX.active, &GFX.transform);
}

void gfx_identity ()
{
  gfx_m4_ident(GFX.active);
}

void gfx_perspective (float fov, float a, float n, float f)
{
  fov = ((fov * PI) / 180.0f);
  gfx_m4_perspective(GFX.active, fov, a, n, f);
}

void gfx_draw_line (float x1, float y1, float x2, float y2, u32 color)
{
  int count = 0, max;
  int width = GFX.target_width;

  u32 x1f, x2f, y1f, y2f;
  u32 fxstep, fystep;

  float xdiff = x2 - x1;
  float ydiff = y2 - y1;

  float xstep = xdiff == 0 ? 0 : xdiff < 0 ? -1 : 1;
  float ystep = ydiff == 0 ? 0 : ydiff < 0 ? -1 : 1;

  float axdiff = fabs(xdiff);
  float aydiff = fabs(ydiff);

  if (axdiff > aydiff) {
    ystep = ydiff / axdiff;
    max = axdiff;
  } else {
    xstep = xdiff / aydiff;
    max = aydiff;
  }

  x1f = gfx_fixed16(x1);
  y1f = gfx_fixed16(y1);

  x2f = gfx_fixed16(x2);
  y2f = gfx_fixed16(y2);

  fxstep = gfx_fixed16(xstep);
  fystep = gfx_fixed16(ystep);

  do {
    /* can this be made faster without being ridiculous? */
    int idx = (y1f>>GFX_FRACBITS) * width + (x1f>>GFX_FRACBITS);

    GFX.target[idx] = color;

    x1f += fxstep;
    y1f += fystep;
  } while (++count < max);
}

void gfx_draw_arrays (int start, int end)
{
  gfxv2 *v1, *v2, *v3;
  gfxm4 mv, mvp;
  gfxv4 tmp;
  int i, vidx, vsize, isize, csize;
  gfxvert *pipe = &GFX.vertex_pipe;

  vsize = GFX.vertex_count * 3;
  isize = end == -1 ? GFX.index_count * 3 : (end + 1) * 3;
  vidx = 0;

  gfx_m4_mult(&mv, &GFX.view, &GFX.model);
  gfx_m4_mult(&mvp, &GFX.projection, &mv);

  for (i = 0; i < vsize; i+=3) {
    gfx_v4_init(&tmp, GFX.vertices[i], GFX.vertices[i+1], GFX.vertices[i+2], 1);
    gfx_v4_mult(&pipe[vidx].camera_space, &tmp, &mvp);
    gfx_project_to_screen(&pipe[vidx].screen_space, &pipe[vidx++].camera_space);
  }

  for (i = start; i < isize; i+=3) {
    v1 = &GFX.vertex_pipe[GFX.indices[i+0]].screen_space;
    v2 = &GFX.vertex_pipe[GFX.indices[i+1]].screen_space;
    v3 = &GFX.vertex_pipe[GFX.indices[i+2]].screen_space;

    gfx_draw_line(v1->x, v1->y, v2->x, v2->y, 0xffffffff);
    gfx_draw_line(v2->x, v2->y, v3->x, v3->y, 0xffffffff);
    gfx_draw_line(v3->x, v3->y, v1->x, v1->y, 0xffffffff);
  }
}

void gfx_draw_text_8x8 (char font[][8], const char *str, int length, int offx, int offy)
{
  int i, y, x, l, min = 0, max = length;
  unsigned int c;
  unsigned int *buf = GFX.target;
  int w = GFX.target_width;
  int h = GFX.target_height;

  for (y = 0; y < 8; y++) {
    int idx = (offy + y) * w;
    x = offx;
    if (offy + y < 0 || offy + y >= h) continue;
    for (l = min; l < max; l++) {
      char* letter = (char *)font[(int)str[l]];
      char bits = letter[y];
      for (i = 0; i < 8; i++) {
        if (bits & (1 << i)) c = 0xffffffff; else c = 0;
        if (x >= 0 && x < w) buf[idx + (x++)] = c; else x++;
      }
    }
  }
}

#endif
#endif
