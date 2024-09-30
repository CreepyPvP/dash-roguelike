#pragma once

#include "memory.h"

struct FileRead
{
    u64 bytes_read;
    u8 *memory;
};

FileRead ReadFile(const char *filename, Arena *arena);


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

bool IsKeyDown(Key key);
bool IsKeyJustDown(Key key);

f32 GetTime();

struct Vertex
{
    V2 position;
    V3 color;
};

struct DebugRay
{
    V2 p0;
    V2 p1;
};

struct RenderData
{
    struct 
    {
        Mat4 projection;
        Mat4 view;
    } ubo;
    
    u32 debug_ray_count;
    DebugRay debug_rays[64];

    u32 vertex_count;
    Vertex *vertex_buffer;


};

extern RenderData *render_data;
