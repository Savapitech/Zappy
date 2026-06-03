#pragma once

#include <cmath>

namespace Zappy::Math
{
    class vec3 {
        public:
            float x, y, z;
            vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    };

    class mat4 {
        public:
            float m[16];

            mat4() {
                for (int i = 0; i < 16; ++i) m[i] = 0.0f;
                m[0] = 1.0f;
                m[5] = 1.0f;
                m[10] = 1.0f;
                m[15] = 1.0f;
            }

            const float* value_ptr() const { return m; }
    };

    inline mat4 operator*(const mat4& left, const mat4& right) {
        mat4 res;

        for (int i = 0; i < 16; i++)
            res.m[i] = 0.0f;

        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                res.m[col * 4 + row] =
                    left.m[0 * 4 + row] * right.m[col * 4 + 0] +
                    left.m[1 * 4 + row] * right.m[col * 4 + 1] +
                    left.m[2 * 4 + row] * right.m[col * 4 + 2] +
                    left.m[3 * 4 + row] * right.m[col * 4 + 3];
            }
        }
        return res;
    }

    inline mat4 translate(const mat4& base, const vec3& v) {
        mat4 t;
        t.m[12] = v.x;
        t.m[13] = v.y;
        t.m[14] = v.z;
        return base * t;
    }

    inline mat4 scale(const mat4& base, const vec3& v) {
        mat4 s;
        s.m[0] = v.x;
        s.m[5] = v.y;
        s.m[10] = v.z;
        return base * s;
    }

    inline float radians(float degrees) {
        return degrees * 0.017453292519943295f;
    }

    inline mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
        mat4 res;
        for (int i = 0; i < 16; ++i) res.m[i] = 0.0f;

        float tanHalfFovy = std::tan(fovy / 2.0f);

        res.m[0] = 1.0f / (aspect * tanHalfFovy);
        res.m[5] = 1.0f / (tanHalfFovy);
        res.m[10] = -(zFar + zNear) / (zFar - zNear);
        res.m[11] = -1.0f;
        res.m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);

        return res;
    }
}