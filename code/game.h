#pragma once

#include "game_math.h"
#include "memory.h"
#include "platform.h"

#include <assert.h>


struct Player
{
    u32 flags;

    V2 smooth_position;
    V2 smooth_velocity;
    V2 target_position;
    V2 target_velocity;

    i32 chunk_x;
    i32 chunk_y;
};

struct Enemy
{
    u32 flags;
    V2 position;
};

struct Tower
{
    i32 tile_x;
    i32 tile_y;
};

struct Chunk
{
    u32 width;
    u32 height;

    u32 enemy_count;
    Enemy enemies[64];

    u32 tower_count;
    Tower towers[64];
};

struct World
{
    i32 width;
    i32 height;
    Chunk *chunks;
};

struct GameState
{
    Arena memory;
    Arena world_memory;
    RenderData render_data;

    Player player;
    World *world;
};

extern GameInput *input;
extern GameState *state;
