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
    return input->key_states && (1 << key) && !(input->prev_key_states && (1 << key));
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
    u32 primitive_count;
    u32 *offsets;
    u32 *vertex_counts;
};

struct RenderData
{
    MultiDraw debug_rays;
    MultiDraw level;
    MultiDraw enemies;

    u32 vertex_count;
    Vertex *vertex_buffer;
};
