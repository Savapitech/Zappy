#include <criterion/criterion.h>
#include <cmath>

#include "Utils/math.hpp"

using namespace Zappy::Math;

static const float EPS = 1e-5f;

Test(vec3, default_is_zero) {
  vec3 v;
  cr_assert_float_eq(v.x, 0.f, EPS);
  cr_assert_float_eq(v.y, 0.f, EPS);
  cr_assert_float_eq(v.z, 0.f, EPS);
}

Test(vec3, subtraction_is_componentwise) {
  vec3 a(4, 6, 8), b(1, 2, 3);
  vec3 c = a - b;
  cr_assert_float_eq(c.x, 3.f, EPS);
  cr_assert_float_eq(c.y, 4.f, EPS);
  cr_assert_float_eq(c.z, 5.f, EPS);
}

Test(vec3, dot_of_orthogonal_axes_is_zero) {
  cr_assert_float_eq(dot(vec3(1, 0, 0), vec3(0, 1, 0)), 0.f, EPS);
  cr_assert_float_eq(dot(vec3(0, 1, 0), vec3(0, 0, 1)), 0.f, EPS);
}

Test(vec3, dot_of_parallel_is_squared_length) {
  cr_assert_float_eq(dot(vec3(2, 0, 0), vec3(2, 0, 0)), 4.f, EPS);
  cr_assert_float_eq(dot(vec3(1, 2, 2), vec3(1, 2, 2)), 9.f, EPS);
}

Test(vec3, length_of_345_triangle) {
  cr_assert_float_eq(length(vec3(3, 4, 0)), 5.f, EPS);
  cr_assert_float_eq(length(vec3(1, 2, 2)), 3.f, EPS);
}

Test(vec3, normalize_yields_unit_length) {
  vec3 n = normalize(vec3(0, 3, 4));
  cr_assert_float_eq(length(n), 1.f, EPS);
}

Test(vec3, normalize_zero_stays_zero) {
  vec3 n = normalize(vec3(0, 0, 0));
  cr_assert_float_eq(length(n), 0.f, EPS);
}

Test(vec3, cross_of_x_and_y_is_z) {
  vec3 c = cross(vec3(1, 0, 0), vec3(0, 1, 0));
  cr_assert_float_eq(c.x, 0.f, EPS);
  cr_assert_float_eq(c.y, 0.f, EPS);
  cr_assert_float_eq(c.z, 1.f, EPS);
}

Test(vec3, cross_is_anticommutative) {
  vec3 a(1, 2, 3), b(4, 5, 6);
  vec3 ab = cross(a, b), ba = cross(b, a);
  cr_assert_float_eq(ab.x, -ba.x, EPS);
  cr_assert_float_eq(ab.y, -ba.y, EPS);
  cr_assert_float_eq(ab.z, -ba.z, EPS);
}

Test(vec3, cross_of_parallel_is_zero) {
  vec3 c = cross(vec3(2, 2, 2), vec3(4, 4, 4));
  cr_assert_float_eq(length(c), 0.f, EPS);
}

Test(scalar, radians_of_180_is_pi) {
  cr_assert_float_eq(radians(180.f), (float)M_PI, 1e-4f);
}

Test(scalar, radians_of_zero_is_zero) {
  cr_assert_float_eq(radians(0.f), 0.f, EPS);
}

Test(scalar, radians_of_90_is_half_pi) {
  cr_assert_float_eq(radians(90.f), (float)M_PI / 2.f, 1e-4f);
}

Test(transi, t0_returns_a) {
  cr_assert_float_eq(transi(10.f, 20.f, 0.f), 10.f, EPS);
}

Test(transi, t1_returns_b) {
  cr_assert_float_eq(transi(10.f, 20.f, 1.f), 20.f, EPS);
}

Test(transi, midpoint_is_average) {
  cr_assert_float_eq(transi(10.f, 20.f, 0.5f), 15.f, EPS);
}

Test(transi, vec3_interpolates_each_component) {
  vec3 r = transi(vec3(0, 0, 0), vec3(2, 4, 8), 0.5f);
  cr_assert_float_eq(r.x, 1.f, EPS);
  cr_assert_float_eq(r.y, 2.f, EPS);
  cr_assert_float_eq(r.z, 4.f, EPS);
}

Test(mat4, default_is_identity) {
  mat4 m;
  for (int i = 0; i < 16; i++) {
    float expected = (i % 5 == 0) ? 1.f : 0.f;
    cr_assert_float_eq(m.m[i], expected, EPS, "m[%d]", i);
  }
}

Test(mat4, identity_times_identity_is_identity) {
  mat4 a, b;
  mat4 c = a * b;
  for (int i = 0; i < 16; i++) {
    float expected = (i % 5 == 0) ? 1.f : 0.f;
    cr_assert_float_eq(c.m[i], expected, EPS);
  }
}

Test(mat4, translate_sets_translation_column) {
  mat4 base;
  mat4 t = translate(base, vec3(5, 6, 7));
  cr_assert_float_eq(t.m[12], 5.f, EPS);
  cr_assert_float_eq(t.m[13], 6.f, EPS);
  cr_assert_float_eq(t.m[14], 7.f, EPS);
  cr_assert_float_eq(t.m[15], 1.f, EPS);
}

Test(mat4, scale_sets_diagonal) {
  mat4 base;
  mat4 s = scale(base, vec3(2, 3, 4));
  cr_assert_float_eq(s.m[0], 2.f, EPS);
  cr_assert_float_eq(s.m[5], 3.f, EPS);
  cr_assert_float_eq(s.m[10], 4.f, EPS);
}

Test(mat4, value_ptr_points_to_the_data) {
  mat4 m;
  cr_assert_eq(m.value_ptr(), m.m);
}

Test(mat4, rotateZ_zero_angle_is_identity) {
  mat4 r = rotateZ(mat4(), 0.f);
  cr_assert_float_eq(r.m[0], 1.f, EPS);
  cr_assert_float_eq(r.m[5], 1.f, EPS);
}

Test(mat4, rotateZ_quarter_turn_matches_cos_sin) {
  float a = radians(90.f);
  mat4 r = rotateZ(mat4(), a);
  cr_assert_float_eq(r.m[0], std::cos(a), EPS);
  cr_assert_float_eq(r.m[1], std::sin(a), EPS);
  cr_assert_float_eq(r.m[4], -std::sin(a), EPS);
  cr_assert_float_eq(r.m[5], std::cos(a), EPS);
}

Test(mat4, perspective_fills_expected_entries) {
  float fov = radians(45.f), aspect = 16.f / 9.f, zn = 0.1f, zf = 100.f;
  mat4 p = perspective(fov, aspect, zn, zf);
  float th = std::tan(fov / 2.f);
  cr_assert_float_eq(p.m[0], 1.f / (aspect * th), EPS);
  cr_assert_float_eq(p.m[5], 1.f / th, EPS);
  cr_assert_float_eq(p.m[11], -1.f, EPS);
  cr_assert_float_eq(p.m[15], 0.f, EPS);
}

Test(mat4, ortho_fills_expected_entries) {
  mat4 o = ortho(0, 800, 0, 600, -1, 1);
  cr_assert_float_eq(o.m[0], 2.f / 800.f, EPS);
  cr_assert_float_eq(o.m[5], 2.f / 600.f, EPS);
  cr_assert_float_eq(o.m[15], 1.f, EPS);
}

Test(mat4, lookAt_down_negative_z_is_identity_basis) {
  mat4 v = lookAt(vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0));
  cr_assert_float_eq(v.m[0], 1.f, EPS);  // right.x
  cr_assert_float_eq(v.m[5], 1.f, EPS);  // up.y
  cr_assert_float_eq(v.m[10], 1.f, EPS); // -forward.z
  cr_assert_float_eq(v.m[12], 0.f, EPS); // no translation at origin
}

Test(mat4, lookAt_translates_by_eye) {
  mat4 v = lookAt(vec3(0, 0, 5), vec3(0, 0, 0), vec3(0, 1, 0));
  cr_assert_float_eq(v.m[14], -5.f, EPS);
}
