#pragma once

#include "game_math.h"
#include "memory.h"
#include "platform.h"

#include <assert.h>

#define PLAYER_MOVING (1 << 0)

#define ENEMY_DEAD (1 << 0)

#define ROOM_HAS_PLAYER_SPAWN (1 << 0)

// Direction in screen space! 
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

struct Room
{
    u32 flags;

    u8 tiles[256];
    i32 width;
    i32 height;

    u32 enemy_count;
    Enemy enemies[20];

    V2i offset_entrance;
    V2i offset_exit;
    Direction direction_exit;
    Direction direction_entrance;

    V2i player_spawn;
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

    // Assets...
    u32 room_count;
    Room rooms[32];
};

extern GameInput *input;
extern GameState *state;
