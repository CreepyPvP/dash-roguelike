#include <stdio.h>

#include "defines.h"
#include "game.h"
#include "memory.h"
#include "game_math.h"
#include "platform.h"

#include "memory.cpp"
#include "game_math.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// NOTE: Cpp context causes name mangling. sad :(
extern "C"
{
    __declspec(dllexport) RenderData * __stdcall GameUpdate(GameInput *input_data, GameAssets *assets, u8 *memory);
    __declspec(dllexport) void _stdcall GameInitialize(u8 *memory, u64 memory_size);
}

Player *player;

GameInput *input;
GameAssets *assets;
GameState *state;

// Rendering stuff...

struct MultiDrawBuffer
{
    i32 primitive_count;
    i32 offsets[512];
    i32 counts[512];
};

u32 vertex_count;
Vertex vertex_buffer[4096];

MultiDrawBuffer level_buffer;
MultiDrawBuffer debug_buffer;
MultiDrawBuffer entity_buffer;
MultiDrawBuffer player_buffer;

inline MultiDraw BufferToDraw(MultiDrawBuffer *buffer)
{
    MultiDraw draw = {};
    draw.primitive_count = buffer->primitive_count;
    draw.offsets = buffer->offsets;
    draw.counts = buffer->counts;
    return draw;
}

void DrawQuad(MultiDrawBuffer *buffer, V2 topleft, V2 size, V3 color)
{
    assert(vertex_count + 4 <= lengthof(vertex_buffer));
    assert(buffer->primitive_count < lengthof(buffer->offsets));

    Vertex *p0 = &vertex_buffer[vertex_count + 0];
    p0->position = v3(topleft.x, topleft.y, 0);
    p0->normal = v3(0, 0, 1);
    p0->uv = v2(0, 0);
    p0->color = color;

    Vertex *p1 = &vertex_buffer[vertex_count + 1];
    p1->position = v3(topleft.x + size.x, topleft.y, 0);
    p1->normal = v3(0, 0, 1);
    p1->uv = v2(0, 0);
    p1->color = color;

    Vertex *p2 = &vertex_buffer[vertex_count + 2];
    p2->position = v3(topleft.x, topleft.y + size.y, 0);
    p2->normal = v3(0, 0, 1);
    p2->uv = v2(0, 0);
    p2->color = color;

    Vertex *p3 = &vertex_buffer[vertex_count + 3];
    p3->position = v3(topleft.x + size.x, topleft.y + size.y, 0);
    p3->normal = v3(0, 0, 1);
    p3->uv = v2(0, 0);
    p3->color = color;

    u32 primitive = buffer->primitive_count;
    buffer->offsets[primitive] = vertex_count;
    buffer->counts[primitive] = 4;

    vertex_count += 4;
    buffer->primitive_count++;
}

void LoadState()
{
}

void GameInitialize(u8 *memory, u64 memory_size)
{
    state = (GameState *) memory;
    *state = {};

    Arena *arena = (Arena *) memory;
    arena->memory = memory;
    arena->capacity = memory_size;
    arena->offset = sizeof(GameState);

    LoadState();
}

RenderData *GameUpdate(GameInput *input_data, GameAssets *asset_data, u8 *memory)
{
    input = input_data;
    assets = asset_data;
    state = (GameState *) memory;
    f32 delta = input->delta;
    player = &state->player;
    World *world = state->world;

    vertex_count = {};
    level_buffer = {};
    debug_buffer = {};
    entity_buffer = {};
    player_buffer = {};

    if (KeyJustDown(Key_R))
    {
        LoadState();
    }

    // We render at 960 x 540
    // 0,0 ------------> 960,0
    // |
    // | each tile is 32 x 32
    // | 30 x 16.8 tiles
    // | 
    // |
    // 0,540

    RenderData *render = &state->render_data;

    render->mesh_count = 1;
    render->meshes[0] = assets->alien;

    for (u32 y = 0; y < 16; ++y)
    {
        for (u32 x = 0; x < 16; ++x)
        {
            DrawQuad(&level_buffer, v2(x * 32, y * 32), v2(32), v3(0.2, 0.8, 0.2));
        }
    }

    render->vertex_count = vertex_count;
    render->vertex_buffer = vertex_buffer;
    render->debug = BufferToDraw(&debug_buffer);
    render->level = BufferToDraw(&level_buffer);
    render->entities = BufferToDraw(&entity_buffer);
    render->player = BufferToDraw(&player_buffer);

    return render;
}
