#pragma once

#include "defines.h"
#include "memory.h"
#include "game_math.h"

struct FileRead
{
    u64 bytes_read;
    u8 *memory;
};

FileRead ReadFile(const char *filename, Arena *arena);

// Inputs...
//

enum Key
{
    Key_W,
    Key_A,
    Key_S,
    Key_D,
    Key_C,
    Key_R,
    Key_Count,
};

struct GameInput
{
    f32 time;
    f32 delta;
    u32 key_states;
    u32 prev_key_states;
};

extern GameInput *input;

inline bool KeyDown(Key key)
{
    return input->key_states & (1 << key);
}

inline bool KeyJustDown(Key key)
{
    return (input->key_states & (1 << key)) && !(input->prev_key_states && (1 << key));
}

// Renderer api...
//

struct Vertex
{
    V2 position;
    V3 color;
};

struct MultiDraw
{
    i32 primitive_count;
    i32 *offsets;
    i32 *counts;
};

struct SingleDraw
{
    i32 offset;
    i32 count;
};

struct RenderData
{
    MultiDraw debug;
    MultiDraw level;
    MultiDraw enemies;
    MultiDraw player;

    u32 vertex_count;
    Vertex *vertex_buffer;
};

typedef RenderData *GameUpdateCall(GameInput *input, u8 *memory);
typedef void GameInitializeCall(u8 *memory, u64 memory_size);
