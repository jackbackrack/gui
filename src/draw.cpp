#include "glut.h"
#include "draw.h"
#include <stdlib.h>
#include <string.h>

color_t black;
color_t white;

letter_t::letter_t () { }

void draw_fat_lines (std::vector<vec_t<2> >* points, float lw) {
  vec_t<2> lv = points->at(0);
  for (int i = 1; i < points->size(); i++) {
    vec_t<2> vs, vl;
    vec_t<2> nv = points->at(i);
    if ((lv.y == nv.y && lv.x < nv.x) || (lv.x == nv.x && lv.y < nv.y)) { 
      vs = lv;  vl = nv;
    } else {
      vs = nv;  vl = lv;
    }
    vec_t<2> v1 = vs - lw;
    vec_t<2> v2 = vl + lw;
    glRectf(v1.x, v1.y, v2.x, v2.y);
    lv = nv;
  }
}

void draw_lines (std::vector<vec_t<2> >* points, int mode) {
  glBegin(mode);
  for (int i = 0; i < points->size(); i++)
    gl_vertex(points->at(i));
  glEnd();
}

letter_t* letters[N_LETTERS];
// letter_t* fat_letters[N_LETTERS];

struct letter_code_t {
  char     c;
  char*    codes;
};

// #define AMBIGUOUS

letter_code_t letter_codes[] =
{
#ifdef AMBIGUOUS
 {'0', "LBLTRTRBLB;"},
#else
 {'0', "LBLTRTRBLB;CTCB;"},
#endif
 {'1', "CTCB;"},
 // {'2', "LTCTCCLCLBRB;"}, // narrower top
 {'2', "LTRTRCLCLBRB;"}, // straight up
 {'3', "LTRTRBLB;LCRC;"},
 {'4', "LTLCRC;RTRB;"},
 {'5', "LBRBRCLCLTRT;"},
 {'6', "LCRCRBLBLTRT;"},
 {'7', "LTRTRB;"},
 {'8', "LBLTRTRBLB;LCRC;"},
 {'9', "LBRBRTLTLCRC;"},

 {'A', "LBLTRTRB;LCRC;"},
#ifdef AMBIGUOUS
 {'B', "LBLTRTRBLB;LCRC;"},      // like 8
#else
 {'B', "LBLTRTRBLB;LCRC;CTCC;"},
#endif
 // {'B', "LBLTCTCC;LCRCRBLB;"}, // narrower top
 {'C', "RBLBLTRT;"},
 // {'D', "LBLCCCCTRTRBLBLC;"}, // missing upper left
#ifdef AMBIGUOUS
 {'D', "LBLTRTRBLB;"}, // like O
#else
 {'D', "LBLTRTRBLB;LCCCCT;"}, // extra square up top
#endif
 {'E', "RTLTLBRB;LCRC;"},
 {'F', "LBLTRT;LCRC;"},
 {'G', "RTLTLBRBRCCC;"},
 {'H', "LTLB;LCRC;RTRB;"},
 // {'I', "CTCB;"},
 {'I', "LTRT;CTCB;LBRB;"},
 {'J', "LCLBRBRT;"},
#ifdef AMBIGUOUS
 {'K', "LTLB;LCRC;RTRB;"}, // like H
#else
 {'K', "LTLB;LCRC;RTRB;CTCC;"}, // like H with extra top vert
#endif
 // {'K', "LTLB;LCCCCT;CCRCRB;"}, // missing upper right
 {'L', "LTLBRB;"},
 {'M', "LBLTRTRB;CTCB;"},
 {'N', "LBLTRTRB;"},
 {'O', "LBLTRTRBLB;"},
 {'P', "LBLBLTRTRCLC;"},
 {'Q', "LBLTRTRBLB;CBCC;"},
#ifdef AMBIGUOUS
 {'R', "LBLTRTRB;LCRC;"}, // like A
#else
 {'R', "LBLTRTRB;LCRC;CTCC;"}, // like A with extra top vert
#endif
 // {'R', "LBLTCTCCRCRB;LCRC;"}, // missing upper right 
 {'S', "LBRBRCLCLTRT;"},
 {'T', "LTRT;CTCB;"},
 {'U', "LTLBRBRT;"},
#ifdef AMBIGUOUS
 {'V', "LTLBRBRT;"}, // like U
#else
 {'V', "LTLBRBRT;LCCCCB;"}, 
#endif
 // {'V', "LTLBCBCCRCRT;"}, // missing lower left
 {'W', "LTLBRBRT;CTCB;"},
 // {'X', "LTLCCC;CTCB;CCRCRB;"}, // leaning
#ifdef AMBIGUOUS
 {'X', "LTLB;LCRC;RTRB;"}, // like H
#else
 {'X', "LTLB;LCRC;RTRB;CBCT;"}, // like H with extra vert
#endif
 // {'Y', "LTLCRCRT;CCCB;"}, // STRAIGHT UP Y
 {'Y', "LTLCRC;RTRBLB;"}, // y WITH TAIL
#ifdef AMBIGUOUS
 {'Z', "LTRTRCLCLBRB;"}, // like 2
#else
 {'Z', "LTRTRCLCLBRB;CTCB;"},
#endif

 // {'a', "LBLCRCRBLB;LTRTRC;"}, // biggee
 // {'a', "LCRCRBCBCC;"},
 {'a', "LBCBCCRCRBCB;"},
 // {'a', "LTRTRBCBCCRC;"}, // wrapping lowercase a
 // {'b', "LTLBRBRCLC;"}, // big o
 {'b', "LTCTCBRBRCCC;"},
 // {'c', "RBLBLCRC;"}, // big o
 // {'c', "LCRC;CCCBRB;"}, // lil o
 {'c', "LBCBCCRC;CBRB;"},
 // {'d', "RTRBLBLCRC;"}, // big o
 {'d', "RTCTCBLBLCCC;"},
 // {'e', "RBLBLTRTRCLC;"}, // biggee
 {'e', "RBLBLCCCCB;"},
 {'f', "CBCTRT;LCRC;"},
 // {'g', "LBRBRTLTLCRC;"}, // biggee
 // {'g', "LBRBRCCCCB;"},
 {'g', "LBRBRTCTCCRC;"},
 // {'h', "LTLB;LCRCRB;"}, // big o
 {'h', "LTCTCB;CCRCRB;"},
 {'i', "CBCC;"},
 // {'j', "LBCBCT;LCRC;"},
 {'j', "LCLBCBCTRT;"},
 {'k', "LTLB;LCRC;CCCB;"},
 // {'l', "LTCTCB;"},
 {'l', "LBRB;CBCT;"},
 {'m', "LBLCRCRB;CBCC;"},
 // {'n', "LBLCRCRB;"}, // big o
 {'n', "LCRCRB;CCCB;"},
 // {'o', "LBLCRCRBLB;"}, // big o
 {'o', "LCRCRBCBCC;"},
 {'p', "LBCBCTRTRCCC;"},
 {'q', "RBCBCTLTLCCC;"},
 // {'r', "LBLCRC;"}, big o
 {'r', "LBCBCCRCRB;"},
 {'s', "LBCBCCRC;"},
 {'t', "CTCBRB;LCRC;"},
 {'u', "LCLBCBCC;CBRB;"},
 {'v', "LCLBCBCCRC;"},
 {'w', "LCLBRBRC;CCCB;"},
 {'x', "LCRC;LBRB;CCCB;"},
 // {'y', "LTLCRC;RTRBLB;"}, // biggee
 {'y', "LBRBRC;CCCB;"},
 {'z', "LCCCCBRB;"},

 // {'*', "LCRC;CTCB;LBRT;LTRB;"}, // diagonal
 {'*', "LBLTRTRBLB;LCRC;CTCB;"}, // O and +
 {'+', "LCRC;CTCB;"}, 

 // {'%', "LCLTCTCCLC;LBRT;CBCCRCRBCB;"}, // diagonal
 {'%', "LCLTCTCCLC;CBCCRCRBCB;"}, // minus diagonal

 {' ', ";"},

 {'-', "LCRC;"},
 {'=', "LBRB;LTRT;"},
 {'|', "CTCB;"},
 {'?', "LTRTRCCCCB;"},
 // {'!', "CTCC;CB."}, // point
 {'!', "CTCC;LBRB;"}, // squared off bottom
 // {',', "CCLB;"}, // diagonal
 {',', "CCCBLB;"},  // squared off
 // {'.', "CB."}, // point
 {'.', "LBRB;CBCC;"}, // squared off bottom
 {':', "CB.CT."}, // point
 {':', "LTRT;LCRC;LBRB;"}, // squared off looks like equiv
 // {';', "CCLB;CT."}, // diagonal
 {';', "CTCC;LBCB;"}, // squared off
 // {'/', "LBRT;"}, // diagonal
 {'/', "LBCBCTRT;"}, // squared off
 // {'\\', "LTRB;"}, // diagonal
 {'\\', "LTCTCBRB;"}, // squared off
 {'_', "LBRB;"},

 // {'<', "CTLTLBCB;"},
 // {'>', "CBRBRTCT;"},
 // angled versions
 // {'<', "RBLCRT;"},
 // {'>', "LBRCLT;"},
 {'<', "RTCTCBRB;LCCC;"},
 {'>', "LTCTCBLB;CCRC;"},
 {'[', "CTLTLBCB;"},
 {']', "CBRBRTCT;"},
 {'{', "CBLBLTCT;LCCC;"},
 {'}', "CBRBRTCT;CCRC;"},
 {'@', "RBLBLTRTRCCCCT;"}, 
 {0,   ""}
};

struct letter_mag_spec_t {
  char code;
  int  mag;
};

letter_mag_spec_t letter_mag_specs[] =
  {{'L', -80}, {'C', 0}, {'R',  80}, 
   {'B', -80}, {'C', 0}, {'T',  80},
   {0, 0}};

int letter_mags[N_LETTERS];

int read_points_into
   (int j, int *xmags, int *ymags, letter_code_t& code, letter_lines_t* lines) {
  char* cs = code.codes;
  int sx = 0, sy = 0;
  int lx = 0, ly = 0;
  int cx = 0, cy = 0;
  vec_t<2> pp;
  for (; j < strlen(cs); ) {
    char c = cs[j++];
    if (c == '.' || c == ':' || c == ';') {
      if (c == '.') 
        lines->points.push_back(vec(pp.x, pp.y * 0.8));
      lines->kind = c;
      // printf("<%d %c>", k, c);
      return j;
    }
    cx = xmags[c];
    // printf("%c(%d)", c, cx);
    c  = cs[j++];
    cy = ymags[c];
    // printf("%c(%d)", c, cy);
    pp = vec(cx / 200.0, cy / 200.0);
    lines->points.push_back(pp);
  }
  return 100000;
}

void copy_lower_case_letters () {
  for (int c = 'a'; c <= 'z'; c++) {
    letter_t* src = letters[toupper(c)];
    letter_t* dst = new letter_t();
    dst->c = c;
    for (int i = 0; i < src->lines.size(); i++) {
      letter_lines_t* srclines = src->lines[i];
      letter_lines_t* dstlines = new letter_lines_t();
      dst->lines.push_back(dstlines);
      dstlines->kind = srclines->kind;
      for (int j = 0; j < srclines->points.size(); j++) {
        vec_t<2> sp = srclines->points[j];
        dstlines->points.push_back(vec(sp.x, sp.y * 0.8 - 0.2));
      }
    }
  }
}

void init_letters_aux
    (letter_code_t* letter_codes, letter_t** letters, 
     letter_mag_spec_t* xmagspecs, letter_mag_spec_t* ymagspecs, 
     int* xmags, int* ymags) {
  for (int i = 0; i < N_LETTERS; i++) {
    letters[i] = new letter_t();
    ymags[i] = 0;
    xmags[i] = 0;
  }
  for (int i=0; ; i++) {
    char c = xmagspecs[i].code;
    if (c == 0) break;
    xmags[c] = xmagspecs[i].mag;
  }
  for (int i=0; ; i++) {
    char c = ymagspecs[i].code;
    if (c == 0) break;
    ymags[c] = ymagspecs[i].mag;
  }
  for (int i=0; ; i++) {
    letter_code_t code = letter_codes[i];
    char   c           = code.c;
    letter_t* letter   = letters[c];
    letter->c          = c;
    // printf("\nReading %d %c: ", i, c);
    if (c == 0)
      break;
    else {
      // printf("READING %c ", c);
      for (int j = 0, k = 0; k < strlen(code.codes); j++) {
        letter->lines.push_back(new letter_lines_t());
	k = read_points_into(k, xmags, ymags, code, letter->lines[j]);
	// printf("%d/%d ", letter->lines[j].n, k);
      }
      // printf("-> %d\n", j);
    }
    // debug_fat_letter(c, letter);
  }
  copy_lower_case_letters();
}

void draw_letter (letter_t* let, color_t c, color_t oc, bool is_fill) {
  // printf("LET %c: ", let->c);
  if (is_fill) {
    for (int i = 0; i < let->lines.size(); i++) {
      letter_lines_t* lines = let->lines[i];
      glColor4f(oc[0], oc[1], oc[2], oc[3]);
      draw_fat_lines(&lines->points, 0.2);
    }
    for (int i = 0; i < let->lines.size(); i++) {
      letter_lines_t* lines = let->lines[i];
      glColor4f(c[0], c[1], c[2], c[3]);
      draw_fat_lines(&lines->points, 0.1);
    }
  } else {
    for (int i = 0; i < let->lines.size(); i++) {
      letter_lines_t* lines = let->lines[i];
      glColor4f(c[0], c[1], c[2], c[3]);
      draw_lines(&lines->points, GL_LINE_STRIP);
    }
  }
}

void draw_letter_into (Imath::M44d m, char c, std::vector< std::vector<vec_t<2> > >* polygons) {
  letter_t* let = letters[c];
  // post("LET %c\n", c);
  for (int j = 0; j < let->lines.size(); j++) {
    letter_lines_t* lines = let->lines[j];
    std::vector< vec_t<2> > polygon;
    for (int i = 0; i < lines->points.size(); i++) {
      vec_t<2> vv = lines->points[i];
      Imath::V3d v = Imath::V3d(vv.x, vv.y, 0);
      Imath::V3d tv = v * m;
      // post("[%f %f] ", tv.x, tv.y);
      polygon.push_back(vec(tv.x, tv.y));
    }
    polygons->push_back(polygon);
    // post("\n");
  }
}

void draw_font_letter (char c, bool is_fill) {
  draw_letter(letters[c], white, white, is_fill);
}

void draw_font_string (char* string, bool is_fill) {
  glPushMatrix();
  for (int i = 0; i < strlen(string); i++) {
    draw_font_letter(string[i], is_fill);
    glTranslatef(1, 0, 0);
  }
  glPopMatrix();
}

void draw_string_into (Imath::M44d m, char* string, std::vector< std::vector<vec_t<2> > >* polygons) {
  Imath::M44d nm = m;
  for (int i = 0; i < strlen(string); i++) {
    draw_letter_into(nm, string[i], polygons);
    nm.translate(Imath::V3d(1, 0, 0));
  }
}

void init_draw () {
  black = new float[4];
  white = new float[4];
  black[0] = black[1] = black[2] = 0; black[3] = 1;
  white[0] = white[1] = white[2] = 1; white[3] = 1;
  init_letters_aux(letter_codes, letters, 
                   letter_mag_specs, letter_mag_specs, letter_mags, letter_mags);
}

