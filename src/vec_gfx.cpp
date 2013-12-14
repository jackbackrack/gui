#include "vec.h"
#include "glut.h"

void gl_vertex(const vec_t<2>& v) {
  glVertex2d(v.x, v.y);
}

void gl_vertex(const vec_t<3>& v) {
  glVertex3d(v.x, v.y, v.z);
}

void gl_normal(const vec_t<3>& v) {
  glNormal3d(v.x, v.y, v.z);
}

void gl_color(const vec_t<3>& v) {
  glColor3d(v.x, v.y, v.z);
}

void gl_color(const vec_t<4>& v) {
  glColor4d(v.x, v.y, v.z, v.a);
}

void gl_translate(const vec_t<2>& v) {
  glTranslated(v.x, v.y, 0);
}

void gl_translate(const vec_t<3>& v) {
  glTranslated(v.x, v.y, v.z);
}

void gl_scale(const vec_t<3>& v) {
  glScaled(v.x, v.y, v.z);
}

void gl_scale(const vec_t<2>& v) {
  glScaled(v.x, v.y, 1.0);
}

