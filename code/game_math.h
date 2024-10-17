#pragma once

#include "defines.h"

f32 Floor(f32 a);

inline f32 Min(f32 a, f32 b)
{
    return a < b ? a : b;
}

inline f32 Max(f32 a, f32 b)
{
    return a < b ? b : a;
}

inline f32 Abs(f32 a)
{
    return a > 0 ? a : -a;
}

inline f32 Round(f32 a)
{
    return Floor(a + 0.5);
}

struct V2i
{
    i32 x;
    i32 y;
};

inline V2i v2i(i32 x, i32 y)
{
    return {x, y};
}

struct V2
{
    f32 x;
    f32 y;

    V2 operator*(f32 t);
    V2 operator/(f32 t);
    V2 operator+(V2 b);
    V2 operator-(V2 b);
    void operator+=(V2 b);
};

inline V2 v2(f32 x, f32 y)
{
    return {x, y};
}

inline V2 v2(f32 x)
{
    return {x, x};
}

struct V3
{
    f32 x;
    f32 y;
    f32 z;

    V3 operator/(f32 t);
    V3 operator-(V3 b);
    void operator+=(V3 b);
};

inline V3 v3(f32 x, f32 y, f32 z)
{
    return {x, y, z};
}

inline V3 v3(f32 x)
{
    return {x, x, x};
}

inline f32 Dot(V3 a, V3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline V3 Cross(V3 a, V3 b)
{
    V3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

V2 Norm(V2 a);

V3 Norm(V3 a);

struct Mat4
{
    f32 v[16];
    
    Mat4 operator*(Mat4 b);
};

Mat4 Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
Mat4 LookAt(V3 eye, V3 target, V3 up);

bool AABBCollision(V2 bl0, V2 tr0, V2 bl1, V2 tr1);
