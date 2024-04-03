#pragma once

#include <stdint.h>
#include <cmath>
#include <type_traits>
#include "log.h"

namespace stage {
namespace backstage {

template<typename T>
struct stage_vec3;

template<typename T>
struct stage_vec2;

/* Type Definitions */
template<typename T>
struct stage_vec4 {
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        T v[4];
    };

    stage_vec4() = default;
    template<typename S>
    stage_vec4(S val) : x(val), y(val), z(val), w(val) {}
    template<typename S>
    stage_vec4(S x, S y, S z, S w) : x(x), y(y), z(z), w(w) {}
    template<typename S>
    stage_vec4(const stage_vec3<S>& val, S w) : x(val.x), y(val.y), z(val.z), w(w) {}
    T& operator[](uint32_t id) { return v[id]; }
    T operator[](uint32_t id) const { return v[id]; }
    bool operator==(const stage_vec4<T>& b) { return x == b.x && y == b.y && z == b.z && w == b.w; }
    bool operator!=(const stage_vec4<T>& b) { return !(*this == b); }
    friend stage_vec4 operator+(stage_vec4 a, const stage_vec4& b) { return stage_vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
    friend stage_vec4 operator-(stage_vec4 a, const stage_vec4& b) { return stage_vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
    friend stage_vec4 operator*(stage_vec4 a, const stage_vec4& b) { return stage_vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
};

template<typename T>
struct stage_vec3 {
    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        T v[3];
    };

    stage_vec3() = default;
    template<typename S>
    stage_vec3(S val) : x(val), y(val), z(val) {}
    template<typename S>
    stage_vec3(S x, S y, S z) : x(x), y(y), z(z) {}
    template<typename S>
    stage_vec3(const stage_vec2<S>& val, S z) : x(val.x), y(val.y), z(z) {}
    template<typename S>
    stage_vec3(const stage_vec4<S>& val) : x(val.x), y(val.y), z(val.z) {}
    T& operator[](uint32_t id) { return v[id]; }
    T operator[](uint32_t id) const { return v[id]; }
    bool operator==(const stage_vec3<T>& b) { return x == b.x && y == b.y && z == b.z; }
    bool operator!=(const stage_vec3<T>& b) { return !(*this == b); }
    friend stage_vec3 operator+(stage_vec3 a, const stage_vec3& b) { return stage_vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
    friend stage_vec3 operator-(stage_vec3 a, const stage_vec3& b) { return stage_vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
    friend stage_vec3 operator*(stage_vec3 a, const stage_vec3& b) { return stage_vec3(a.x * b.x, a.y * b.y, a.z * b.z); }
};

template<typename T>
struct stage_vec2 {
    union {
        struct { T x, y; };
        struct { T r, g; };
        T v[2];
    };

    stage_vec2() = default;
    template<typename S>
    stage_vec2(S val) : x(val), y(val) {}
    template<typename S>
    stage_vec2(S x, S y) : x(x), y(y) {}
    template<typename S>
    stage_vec2(const stage_vec3<S>& val) : x(val.x), y(val.y) {}
    T& operator[](uint32_t id) { return v[id]; }
    T operator[](uint32_t id) const { return v[id]; }
    bool operator==(const stage_vec2<T>& b) { return x == b.x && y == b.y; }
    bool operator!=(const stage_vec2<T>& b) { return !(*this == b); }
    friend stage_vec2 operator+(stage_vec2 a, const stage_vec2& b) { return stage_vec2(a.x + b.x, a.y + b.y); }
    friend stage_vec2 operator-(stage_vec2 a, const stage_vec2& b) { return stage_vec2(a.x - b.x, a.y - b.y); }
    friend stage_vec2 operator*(stage_vec2 a, const stage_vec2& b) { return stage_vec2(a.x * b.x, a.y * b.y); }
};

template<typename T>
struct stage_mat4 {
    union
    { 
        struct {
            T m00, m01, m02, m03;
            T m10, m11, m12, m13;
            T m20, m21, m22, m23;
            T m30, m31, m32, m33;
        };
        stage_vec4<T> c[4];
        T v[16];
    };
    
    stage_mat4() = default;
    template<typename S>
    stage_mat4(S val) : m00(val), m11(val), m22(val), m33(val) {}
    template<typename S>
    stage_mat4(const stage_vec4<S>& c0, const stage_vec4<S>& c1, const stage_vec4<S>& c2, const stage_vec4<S>& c3) { c[0] = c0; c[1] = c1, c[2] = c2; c[3] = c3; }
    template<typename S>
    stage_mat4(S m00, S m01, S m02, S m03, S m10, S m11, S m12, S m13, S m20, S m21, S m22, S m23, S m30, S m31, S m32, S m33) :
        m00(m00), m01(m01), m02(m02), m03(m03), m10(m10), m11(m11), m12(m12), m13(m13), m20(m20), m21(m21), m22(m22), m23(m23), m30(m30), m31(m31), m32(m32), m33(m33) {}
    template<typename S>
    stage_mat4(const stage_mat4<S>& other) { for (size_t i = 0; i < 16; i++) v[i] = other.v[i]; }
    stage_vec4<T>& operator[](uint32_t id) { return c[id]; }
    stage_vec4<T> operator[](uint32_t id) const { return c[id]; }
    friend stage_vec4<T> operator*(const stage_mat4<T> m, const stage_vec4<T> val) {
        stage_vec4<T> result (static_cast<T>(0));
        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 4; j++) {
                result[i] += val[j] * m.c[j][i];
            }
        } 
        return result;
    }
    friend stage_mat4<T> operator*(const stage_mat4<T> m, T val) {
        stage_mat4<T> result;
        for (size_t i = 0; i < 16; i++) result.v[i] = m.v[i] * val; 
        return result;
    }
};

typedef stage_vec4<float>       stage_vec4f;
typedef stage_vec3<float>       stage_vec3f;
typedef stage_vec2<float>       stage_vec2f;

typedef stage_mat4<float>       stage_mat4f;

/* Common vector and Matrix Functions */

template<typename T> stage_vec2<T>
make_vec2(T* value_ptr) {
    return stage_vec2<T>(value_ptr[0], value_ptr[1]);
}

template<typename T> stage_vec3<T>
make_vec3(T* value_ptr) {
    return stage_vec3<T>(value_ptr[0], value_ptr[1], value_ptr[2]);
}

template<typename T> stage_vec4<T>
make_vec4(T* value_ptr) {
    return stage_vec4<T>(value_ptr[0], value_ptr[1], value_ptr[2], value_ptr[3]);
}

template<typename T> stage_mat4<T>
make_mat4(T* value_ptr) {
    stage_mat4<T> mat;
    for (size_t i = 0; i < 16; i++) mat.v[i] = value_ptr[i]; 
    return mat;
}

template<typename T> stage_vec3<T>
normalize(const stage_vec3<T>& v) {
    T length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return stage_vec3<T>(v.x / length, v.y / length, v.z / length);
}

template<typename T> stage_vec3<T>
clamp(const stage_vec3<T>& v, T low, T high) {
    return stage_vec3<T>( std::min(high, std::max(low, v.x)), std::min(high, std::max(low, v.y)), std::min(high, std::max(low, v.z)));
}

template<typename T> T
dot(const stage_vec3<T>& a, const stage_vec3<T>& b) {
    return a.x * b.x + a.y + b.y + a.z * b.z;
}

template<typename T> stage_vec3<T>
cross(const stage_vec3<T>& a, const stage_vec3<T>& b) {
    return stage_vec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template<typename T> T
compMax(const stage_vec3<T>& v) {
    T max = v.x > v.y ? v.x : v.y;
    return max > v.z ? max : v.z;
}

template<typename T> T
compMin(const stage_vec3<T>& v) {
    T min = v.x < v.y ? v.x : v.y;
    return min < v.z ? min : v.z;
}

}
}
