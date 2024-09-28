#pragma once

#include "game_math.h"

#include <assert.h>

#define PLAYER_MOVING (1 << 0)

#define ENEMY_DEAD (1 << 0)

enum Direction
{
    Direction_Up,
    Direction_Left,
    Direction_Down,
    Direction_Right,
};

struct Player
{
    u32 flags;
    Direction direction;
    V2 position;
};

struct Enemy
{
    u32 flags;
    V2 position;
};

struct Level
{
    i32 width;
    i32 height;
    u8 tiles[256];

    u32 enemy_count;
    Enemy enemies[64];
};

struct DebugRay
{
    V2 p0;
    V2 p1;
};

extern Player player;
extern Level level;

extern u32 debug_ray_count;
extern DebugRay debug_rays[64];

inline DebugRay *AllocDebugRay()
{
    assert(debug_ray_count < 64);
    return &debug_rays[debug_ray_count++];
}
