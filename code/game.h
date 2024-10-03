#pragma once

#include "game_math.h"
#include "memory.h"
#include "platform.h"

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

struct GameState
{
    Arena memory;
    RenderData render_data;

    Player player;
    Level level;
};

extern GameInput *input;
extern GameState *state;
