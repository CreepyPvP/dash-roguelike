#pragma once

#include "defines.h"

struct V2
{
    f32 x;
    f32 y;
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

V3 Norm(V3 a);

struct Mat4
{
    f32 v[16];
    
    Mat4 operator*(Mat4 b);
};

Mat4 Ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f);
Mat4 LookAt(V3 eye, V3 target, V3 up);

