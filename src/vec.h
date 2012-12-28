//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_VEC__
#define __IS_VEC__

#include "glut.h"
#include "utils.h"
#include <cmath>
#include <cassert>
#include <algorithm>
#include <iostream>

using ::std::abs;
using ::std::ceil;
using ::std::floor;
using ::std::sqrt;
using ::std::min;
using ::std::max;

inline float round(float x) {
  return roundf(x);
}

template<class T> inline T min_abs (T a, T b) {
  T a_a = abs(a);
  T a_b = abs(b);
  return a < b ? a : b;
}

template<class T> inline T sqr (T a) {
  return a * a;
}

template<class T> inline T clip (T x, T mn, T mx) {
  return max(mn, min(mx, x));
}

template<class T> inline int sgn (T x) {
  return x>0 ? 1 : x<0 ? -1 : 0;
}

template<class T> inline int sgn_eps (T x, T eps) {
  return x>eps ? 1 : x<-eps ? -1 : 0;
}

template<class T> struct scalar_policy;
template<> struct scalar_policy<float> { typedef float type; };
template<> struct scalar_policy<double> { typedef double type; };

template<int d,class T=double> class vec_t;

template<class T,int d> struct scalar_policy<vec_t<d,T> > { typedef T type; };

template<class T> class vec_t<2,T> {
 public:
  T x,y;

  vec_t()
    : x(),y() {} 

  vec_t(const T& x, const T& y)
    : x(x),y(y) {}

  T& operator[](int k) {
    assert(0<=k && k<2); 
    return (&x)[k];
  }

  const T& operator[](int k) const {
    assert(0<=k && k<2); 
    return (&x)[k];
  }

  bool operator==(const vec_t& v) const {
    return x == v.x && y == v.y;
  }

  bool operator<=(const vec_t& v) const {
    return x <= v.x && y <= v.y;
  }

  bool operator<(const vec_t& v) const {
    return *this <= v && *this != v;
  }

  bool operator!=(const vec_t& v) const {
    return !(*this == v);
  }

  bool is_same_eps(const vec_t& v, T eps) {
    return abs(v.x - x) <= eps && abs(v.y - y) <= eps;
  }

  vec_t rot(T a) {
    T c = cos(a), s = sin(a);
    return vec_t(x * c - y * s, x * s + y * c);
  }

/*
  TODO: add back
  vec_t operator*(const vec_t& b) {
    return vec_t(x * b.x, y * b.y);
  }
*/

  vec_t mul(const vec_t& b) {
    return vec_t(x * b.x, y * b.y);
  }

  vec_t operator/(const vec_t& b) const {
    return vec_t(x / b.x, y / b.y);
  }

  vec_t operator-(T b) const {
    return vec_t(x - b, y - b);
  }

  vec_t operator-(const vec_t& b) const {
    return vec_t(x - b.x, y - b.y);
  }

  vec_t operator*(T s) const {
    return vec_t(x*s, y*s);
  }

  vec_t operator/(T s) const {
    T inv = 1/s;
    return vec_t(x*inv, y*inv);
  }

  vec_t operator+(const vec_t& b) const {
    return vec_t(x + b.x, y + b.y);
  }

  vec_t operator+(T b) const {
    return vec_t(x + b, y + b);
  }
};

template<class T> vec_t<2,T> vec(const T& x, const T& y) {
  return vec_t<2,T>(x,y);
}

template<class T> vec_t<3,T> vec(const T& x, const T& y, const T& z) {
  return vec_t<3,T>(x,y,z);
}

template<> inline vec_t<2,int> vec_t<2,int>::operator/(int s) const {
  return vec(x/s, y/s);
}

template<class T> class vec_t<3,T> {
 public:
  T x,y,z;

  vec_t()
    : x(),y(),z() {} 

  vec_t(const T& x, const T& y, const T& z)
    : x(x),y(y),z(z) {}

  template<class S> explicit vec_t(const vec_t<3,S>& v)
    : x(v.x), y(v.y), z(v.z) {}

  vec_t(const vec_t<2,T>& v, const T& z)
    : x(v.x), y(v.y), z(z) {}

  T& operator[](int k) {
    assert(0<=k && k<3); 
    return (&x)[k];
  }

  const T& operator[](int k) const {
    assert(0<=k && k<3); 
    return (&x)[k];
  }

  bool operator==(const vec_t& v) const {
    return x == v.x && y == v.y && z == v.z;
  }

  bool operator<=(const vec_t& v) const {
    return x <= v.x && y <= v.y && z <= v.z;
  }

  bool operator<(const vec_t& v) const {
    return *this <= v && *this != v;
  }

  bool operator!=(const vec_t& v) const {
    return !(*this == v);
  }

  bool is_same_eps(const vec_t& v, T eps) const {
    return abs(v.x - x) <= eps && abs(v.y - y) <= eps && abs(v.z - z) <= eps;
  }

  vec_t<2,T> xy() const {
    return vec(x, y);
  }

  vec_t<2,T> yz() const {
    return vec(0, y, z);
  }

  vec_t<2,T> xz() const {
    return vec(x, 0, z);
  }

  int dominant_axis() const {
    T ax = abs(x), ay = abs(y), az = abs(z);
    return ax >= ay
      ? ax >= az ? 0 : 2
      : ay >= az ? 1 : 2;
  }

/*
  TODO: add back
  vec_t operator*(const vec_t& b) {
    return vec_t(x * b.x, y * b.y, z * b.z);
  }
*/

  vec_t mul(const vec_t& b) {
    return vec_t(x * b.x, y * b.y, z * b.z);
  }

  vec_t operator/(const vec_t& b) const {
    return vec_t(x / b.x, y / b.y, z / b.z);
  }

  vec_t operator-(T b) const {
    return vec_t(x - b, y - b, z - b);
  }

  vec_t operator-(const vec_t& b) const {
    return vec_t(x - b.x, y - b.y, z - b.z);
  }

  vec_t operator*(T s) const {
    return vec_t(x*s, y*s, z*s);
  }

/*
  vec_t operator*(const vec_t& b) {
    return vec_t(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x);
  }
*/

  vec_t operator/(T s) const {
    T inv = 1/s;
    return vec_t(x*inv, y*inv, z*inv);
  }

  vec_t operator+(const vec_t& b) const {
    return vec_t(x + b.x, y + b.y, z + b.z);
  }

  vec_t operator+(T b) const {
    return vec_t(x + b, y + b, z + b);
  }

  vec_t rot_dims(int axis) {
    assert(0<=axis && axis<3);
    switch (axis) {
      case 0:
        return vec_t(x, z, y);
      case 1:
        return vec_t(z, y, x);
      case 2:
        return vec_t(y, x, z);
    }
  }

  vec_t rot(int axis, T a) {
    T c = cos(a), s = sin(a);
    switch (axis) {
      case 0:
        return vec(x * c - y * s, x * s + y * c, z);
      case 1:
        return vec(x, y * c - z * s, y * s + z * c);
      case 2:
        return vec(x * c - z * s, y, x * s + z * c);
    }
  }
};

template<> inline vec_t<3,int> vec_t<3,int>::operator/(int s) const {
  return vec(x/s, y/s, z/s);
}

template<class T> class vec_t<4,T> {
 public:
  T x,y,z,a;

  vec_t()
    : x(),y(),z(),a() {} 

  vec_t(const T& x, const T& y, const T& z, const T& a)
    : x(x),y(y),z(z),a(a) {}

  vec_t(const vec_t<2,T>& v, const T& z, const T& a)
    : x(v.x), y(v.y), z(z), a(a) {}

  vec_t(const vec_t<3,T>& v, const T& a)
    : x(v.x), y(v.y), z(v.z), a(a) {}

  T& operator[](int k) {
    assert(0<=k && k<4); 
    return (&x)[k];
  }

  const T& operator[](int k) const {
    assert(0<=k && k<4); 
    return (&x)[k];
  }

  bool operator==(const vec_t& v) const {
    return x == v.x && y == v.y && z == v.z && a == v.a;
  }

  bool operator<=(const vec_t& v) const {
    return x <= v.x && y <= v.y && z <= v.z && a <= v.a;
  }

  bool operator<(const vec_t& v) const {
    return *this <= v && *this != v;
  }

  bool operator!=(const vec_t& v) const {
    return !(*this == v);
  }

  vec_t<2,T> xy() const {
    return vec(x, y);
  }

  vec_t<3,T> xyz() const {
    return vec(x, y, z);
  }

  vec_t mul(const vec_t& b) {
    return vec_t(x * b.x, y * b.y, z * b.z, a * b.a);
  }

  vec_t operator/(const vec_t& b) const {
    return vec_t(x / b.x, y / b.y, z / b.z, a / b.a);
  }

  vec_t operator-(T b) const {
    return vec_t(x - b, y - b, z - b, a - b);
  }

  vec_t operator-(const vec_t& b) const {
    return vec_t(x - b.x, y - b.y, z - b.z, a - b.a);
  }

  vec_t operator*(T s) const {
    return vec_t(x*s, y*s, z*s, a*s);
  }

  vec_t operator/(T s) const {
    T inv = 1/s;
    return vec_t(x*inv, y*inv, z*inv, a*inv);
  }

  vec_t operator+(const vec_t& b) const {
    return vec_t(x + b.x, y + b.y, z + b.z, a + b.a);
  }

  vec_t operator+(T b) const {
    return vec_t(x + b, y + b, z + b, a + b.a);
  }
};

template<class T> vec_t<4,T> vec(const T& x, const T& y, const T& z, const T& a) {
  return vec_t<4,T>(x,y,z,a);
}

template<> inline vec_t<4,int> vec_t<4,int>::operator/(int s) const {
  return vec(x/s, y/s, z/s, a/s);
}

template<class T> inline T dot(const vec_t<2,T>& a, const vec_t<2,T>& b) {
  return a.x*b.x + a.y*b.y;
}

template<class T> inline T dot(const vec_t<3,T>& a, const vec_t<3,T>& b) {
  return a.x*b.x + a.y*b.y + a.z*b.z;
}

template<int d,class T> inline T len(const vec_t<d,T>& v) {
  return sqrt(sqr_len(v));
}

template<class T> inline T sqr_len(const vec_t<2,T>& v) {
  return sqr(v.x) + sqr(v.y);
}

template<class T> inline T sqr_len(const vec_t<3,T>& v) {
  return sqr(v.x) + sqr(v.y) + sqr(v.z);
}

template<class T> inline T sqr_len(const vec_t<4,T>& v) {
  return sqr(v.x) + sqr(v.y) + sqr(v.z) + sqr(v.a);
}

inline void gl_vertex(const vec_t<2>& v) {
  glVertex2d(v.x, v.y);
}

inline void gl_vertex(const vec_t<3>& v) {
  glVertex3d(v.x, v.y, v.z);
}

inline void gl_normal(const vec_t<3>& v) {
  glNormal3d(v.x, v.y, v.z);
}

inline void gl_color(const vec_t<3>& v) {
  glColor3d(v.x, v.y, v.z);
}

inline void gl_color(const vec_t<4>& v) {
  glColor4d(v.x, v.y, v.z, v.a);
}

inline void gl_translate(const vec_t<2>& v) {
  glTranslated(v.x, v.y, 0);
}

inline void gl_translate(const vec_t<3>& v) {
  glTranslated(v.x, v.y, v.z);
}

inline void gl_scale(const vec_t<3>& v) {
  glScaled(v.x, v.y, v.z);
}

template<class T,int d> inline vec_t<d,T> normalize(const vec_t<d,T>& v) {
  T l = len(v);
  if (l)
    return v / l;
  else {
    vec_t<d,T> arbitrary;
    arbitrary[0] = 1;
    return arbitrary;
  }
}

template<class T> inline vec_t<3,T> cross(const vec_t<3,T>& a, const vec_t<3,T>& b) {
  return vec(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template<class T> vec_t<3,T> normal_direction(const vec_t<3,T>& a, const vec_t<3,T>& b, const vec_t<3,T>& c) {
  return cross(b-a, c-a);
}

template<int d,class T> inline T sqr_distance(const vec_t<d,T>& a, const vec_t<d,T>& b) {
  return sqr_len(a - b);
}

template<int d,class T> inline T distance(const vec_t<d,T>& a, const vec_t<d,T>& b) {
  return len(a - b);
}

template<class T,int d> inline vec_t<d,T> operator*(T s, const vec_t<d,T>& v) {
  return v*s;
}

template<class TV> inline TV mean(const TV& a, const TV& b)
{
  typedef typename scalar_policy<TV>::type T;
  return (T).5 * (a + b);
}

template<class TV> inline TV mean(const TV& a, const TV& b, const TV& c)
{
  typedef typename scalar_policy<TV>::type T;
  return T(1./3) * (a + b + c);
}

template<class TV> inline TV mean(const TV& a, const TV& b, const TV& c, const TV& d)
{
  typedef typename scalar_policy<TV>::type T;
  return (T).25 * (a + b + c + d);
}

template<class T> inline vec_t<3,T> round(const vec_t<3,T>& v) {
  return vec_t<3,T>(round(v.x), round(v.y), round(v.z));
}

template<class T> inline vec_t<3,T> ceil(const vec_t<3,T>& v) {
  return vec_t<3,T>(ceil(v.x), ceil(v.y), ceil(v.z));
}

template<class T> inline vec_t<2,T> floor(const vec_t<2,T>& v) {
  return vec_t<2,T>(floor(v.x), floor(v.y));
}

template<class T> inline vec_t<3,T> floor(const vec_t<3,T>& v) {
  return vec_t<3,T>(floor(v.x), floor(v.y), floor(v.z));
}

template<class T> inline vec_t<2,T> pabs(const vec_t<2,T>& v) {
  return vec_t<2,T>(abs(v.x), abs(v.y));
}

template<class T> inline vec_t<3,T> pabs(const vec_t<3,T>& v) {
  return vec_t<3,T>(abs(v.x), abs(v.y), abs(v.z));
}

template<class T> inline vec_t<3,int> psgn(const vec_t<3,T>& v) {
  return vec_t<3,int>(sgn(v.x), sgn(v.y), sgn(v.z));
}

template<class T> inline vec_t<2,T> pmin(const vec_t<2,T>& a, const vec_t<2,T>& b) {
  return vec_t<2,T>(min(a.x, b.x), min(a.y, b.y));
}

template<class T> inline vec_t<3,T> pmin(const vec_t<3,T>& a, const vec_t<3,T>& b) {
  return vec_t<3,T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

template<class T> inline vec_t<4,T> pmin(const vec_t<4,T>& a, const vec_t<4,T>& b) {
  return vec_t<4,T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.a, b.a));
}

template<class T> inline vec_t<2,T> pmax(const vec_t<2,T>& a, const vec_t<2,T>& b) {
  return vec_t<2,T>(max(a.x, b.x), max(a.y, b.y));
}

template<class T> inline vec_t<3,T> pmax(const vec_t<3,T>& a, const vec_t<3,T>& b) {
  return vec_t<3,T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

template<class T> inline vec_t<4,T> pmax(const vec_t<4,T>& a, const vec_t<4,T>& b) {
  return vec_t<4,T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.a, b.a));
}

template<int d,class T> inline vec_t<d,T> bound_by(const vec_t<d,T>& v, const vec_t<d,T>& minv, const vec_t<d,T>& maxv) {
  return pmin(pmax(v,minv),maxv);
}

template<class T> inline T product(const vec_t<2,T>& v) {
  return v.x * v.y;
}

template<class T> inline T product(const vec_t<3,T>& v) {
  return v.x * v.y * v.z;
}

inline void print(const vec_t<2>& v) {
  post("[%.2f,%.2f]", v.x, v.y);
}

inline void print(const vec_t<3>& v) {
  post("[%.2f,%.2f,%.2f]", v.x, v.y, v.z);
}

inline void print(const vec_t<3,int>& v) {
  post("[%d,%d,%d]", v.x, v.y, v.z);
}

template<class T> inline std::ostream& operator<<(std::ostream& output, const vec_t<2,T>& v) {
  output << '[' << v.x << ',' << v.y << ']';
  return output;
}

template<class T> inline std::ostream& operator<<(std::ostream& output, const vec_t<3,T>& v) {
  output << '[' << v.x << ',' << v.y << ',' << v.z << ']';
  return output;
}

#endif
