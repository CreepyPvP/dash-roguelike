#pragma once

#include "game_math.h"
#include "memory.h"
#include "platform.h"

#include <assert.h>


struct Player
{
    u32 flags;

    V2 offset;
    i32 tile_x;
    i32 tile_y;

    i32 chunk_x;
    i32 chunk_y;
};

struct Enemy
{
    u32 flags;
    V2 position;
};

struct Chunk
{
    u32 width;
    u32 height;
    u8 *tiles;

    u32 enemy_count;
    Enemy enemies[64];
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
