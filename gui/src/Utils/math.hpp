#pragma once

#include <cmath>

namespace Zappy::Math {
class vec3 {
public:
  float x, y, z;
  vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
  vec3 operator-(const vec3 &v) const {
    return vec3(x - v.x, y - v.y, z - v.z);
  }
};

inline vec3 cross(const vec3 &a, const vec3 &b) {
  return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x);
}

inline float dot(const vec3 &a, const vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float length(const vec3 &v) { return std::sqrt(dot(v, v)); }

inline vec3 normalize(const vec3 &v) {
  float len = length(v);
  if (len > 0)
    return vec3(v.x / len, v.y / len, v.z / len);
  return v;
}
class mat4 {
public:
  float m[16];

  mat4() {
    for (int i = 0; i < 16; ++i)
      m[i] = 0.0f;
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
  }

  const float *value_ptr() const { return m; }
};

inline mat4 operator*(const mat4 &left, const mat4 &right) {
  mat4 res;

  for (int i = 0; i < 16; i++)
    res.m[i] = 0.0f;

  for (int col = 0; col < 4; col++) {
    for (int row = 0; row < 4; row++) {
      res.m[col * 4 + row] = left.m[0 * 4 + row] * right.m[col * 4 + 0] +
                             left.m[1 * 4 + row] * right.m[col * 4 + 1] +
                             left.m[2 * 4 + row] * right.m[col * 4 + 2] +
                             left.m[3 * 4 + row] * right.m[col * 4 + 3];
    }
  }
  return res;
}

inline mat4 translate(const mat4 &base, const vec3 &v) {
  mat4 t;
  t.m[12] = v.x;
  t.m[13] = v.y;
  t.m[14] = v.z;
  return base * t;
}

inline mat4 scale(const mat4 &base, const vec3 &v) {
  mat4 s;
  s.m[0] = v.x;
  s.m[5] = v.y;
  s.m[10] = v.z;
  return base * s;
}

inline float radians(float degrees) { return degrees * 0.017453292519943295f; }

inline mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
  mat4 res;
  for (int i = 0; i < 16; ++i)
    res.m[i] = 0.0f;

  float tanHalfFovy = std::tan(fovy / 2.0f);

  res.m[0] = 1.0f / (aspect * tanHalfFovy);
  res.m[5] = 1.0f / (tanHalfFovy);
  res.m[10] = -(zFar + zNear) / (zFar - zNear);
  res.m[11] = -1.0f;
  res.m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);

  return res;
}

inline mat4 lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) {
  vec3 f(normalize(center - eye));
  vec3 s(normalize(cross(f, up)));
  vec3 u(cross(s, f));

  mat4 res;
  res.m[0] = s.x;
  res.m[4] = s.y;
  res.m[8] = s.z;
  res.m[1] = u.x;
  res.m[5] = u.y;
  res.m[9] = u.z;
  res.m[2] = -f.x;
  res.m[6] = -f.y;
  res.m[10] = -f.z;

  res.m[12] = -dot(s, eye);
  res.m[13] = -dot(u, eye);
  res.m[14] = dot(f, eye);
  return res;
}

inline mat4 rotateX(const mat4 &base, float angle) {
  mat4 r;
  float c = std::cos(angle);
  float s = std::sin(angle);
  r.m[5] = c;
  r.m[6] = s;
  r.m[9] = -s;
  r.m[10] = c;
  return base * r;
}

inline mat4 rotateY(const mat4 &base, float angle) {
  mat4 r;
  float c = std::cos(angle);
  float s = std::sin(angle);
  r.m[0] = c;
  r.m[2] = -s;
  r.m[8] = s;
  r.m[10] = c;
  return base * r;
}

inline mat4 rotateZ(const mat4 &base, float angle) {
  mat4 r;
  float c = std::cos(angle);
  float s = std::sin(angle);
  r.m[0] = c;
  r.m[1] = s;
  r.m[4] = -s;
  r.m[5] = c;
  return base * r;
}

inline mat4 ortho(float left, float right, float bottom, float top, float zNear,
                  float zFar) {
  mat4 res;
  for (int i = 0; i < 16; i++)
    res.m[i] = 0.0f;

  res.m[0] = 2.0f / (right - left);
  res.m[5] = 2.0f / (top - bottom);
  res.m[10] = -2.0f / (zFar - zNear);
  res.m[12] = -(right + left) / (right - left);
  res.m[13] = -(top + bottom) / (top - bottom);
  res.m[14] = -(zFar + zNear) / (zFar - zNear);
  res.m[15] = 1.0f;
  return res;
}
} // namespace Zappy::Math
