#pragma once

#include <stdint.h>
#include <type_traits>

namespace stage {
namespace backstage {

/* Type Definitions */
template<typename T>
struct stage_vec4 {
    union {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        T v[4];
    };

    stage_vec4() = default;
    stage_vec4(T val) : x(val), y(val), z(val), w(val) {}
    stage_vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    T& operator[](uint32_t id) { return v[id]; }
    bool operator==(const stage_vec4<T>& b) { return x == b.x && y == b.y && z == b.z && w == b.w; }
    bool operator!=(const stage_vec4<T>& b) { return !(*this == b); }
    static stage_vec4<T> make_vec4(T* values) { 
        stage_vec4<T> vec;
        for (size_t i = 0; i < 4; i++) vec.v[i] = values[i]; 
        return vec;
    }
};

template<typename T>
struct stage_vec3 {
    union {
        struct { T x, y, z; };
        struct { T r, g, b; };
        T v[3];
    };

    stage_vec3() = default;
    stage_vec3(T val) : x(val), y(val), z(val) {}
    stage_vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    T& operator[](uint32_t id) { return v[id]; }
    bool operator==(const stage_vec3<T>& b) { return x == b.x && y == b.y && z == b.z; }
    bool operator!=(const stage_vec3<T>& b) { return !(*this == b); }
    static stage_vec3<T> make_vec3(T* values) { 
        stage_vec3<T> vec;
        for (size_t i = 0; i < 3; i++) vec.v[i] = values[i]; 
        return vec;
    }
};

template<typename T>
struct stage_vec2 {
    union {
        struct { T x, y; };
        struct { T r, g; };
        T v[2];
    };

    stage_vec2() = default;
    stage_vec2(T val) : x(val), y(val) {}
    stage_vec2(T x, T y) : x(x), y(y) {}
    bool operator==(const stage_vec2<T>& b) { return x == b.x && y == b.y; }
    bool operator!=(const stage_vec2<T>& b) { return !(*this == b); }
    T& operator[](uint32_t id) { return v[id]; }
    static stage_vec2<T> make_vec2(T* values) { 
        stage_vec2<T> vec;
        for (size_t i = 0; i < 2; i++) vec.v[i] = values[i]; 
        return vec;
    }
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
    
    T& operator[](uint32_t id) { return v[id]; }
    static stage_mat4<T> make_mat4(T* values) { 
        stage_mat4<T> mat;
        for (size_t i = 0; i < 16; i++) mat.v[i] = values[i]; 
        return mat;
    }
};

typedef stage_vec4<float>       stage_vec4f;
typedef stage_vec3<float>       stage_vec3f;
typedef stage_vec2<float>       stage_vec2f;

typedef stage_mat4<float>       stage_mat4f;

/* Common Vector and Matrix Functions */

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

}
}