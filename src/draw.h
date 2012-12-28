#ifndef __IS_DRAW__
#define __IS_DRAW__

#include <math.h>
#include "utils.h"
#include <list>
#include <deque>
#include <vector>
#include "glut.h"
// #include "stl.h"
#include <ImathMatrix.h>
#include <ImathMatrixAlgo.h>
#include <ImathVec.h>
#include "vec.h"

#define MAX_CODES  128

typedef float* color_t ;

class letter_lines_t {
 public:
  char kind;
  std::vector<vec_t<2> > points;
};

#define MAX_LETTER_LINES 4

class letter_t {
 public:
  int c;
  std::vector<letter_lines_t*> lines;
  letter_t ();
};

#define N_LETTERS 256

extern letter_t* letters[N_LETTERS];

void init_draw ();
void draw_letter (letter_t* let, color_t c, color_t oc, bool is_fill=false);
void draw_font_letter (char c, bool is_fill=false);
void draw_font_string (char* s, bool is_fill=false);
void draw_string_into (Imath::M44d m, char* s, std::vector< std::vector<vec_t<2> > >* polygons);
void draw_letter_into (Imath::M44d m, char  c, std::vector< std::vector<vec_t<2> > >* polygons);

extern color_t black;
extern color_t white;

#endif
