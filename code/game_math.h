#pragma once

#include "defines.h"

struct V2
{

    f32 x;
    f32 y;

    // V2(f32 x, f32 y)
    // {
    //     this->x = x;
    //     this->y = y;
    // }

};

inline V2 v2(f32 x, f32 y)
{
    return {x, y};
}

struct V3
{
    f32 x;
    f32 y;
    f32 z;

    // V3(f32 x, f32 y, f32 z)
    // {
    //     this->x = x;
    //     this->y = y;
    //     this->z = z;
    // }

};

inline V3 v3(f32 x, f32 y, f32 z)
{
    return {x, y, z};
}

