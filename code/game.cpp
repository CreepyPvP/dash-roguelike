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
    __declspec(dllexport) RenderData * __stdcall GameUpdate(GameInput *input_data, u8 *memory);
    __declspec(dllexport) void _stdcall GameInitialize(u8 *memory, u64 memory_size);
}

Player *player;

GameInput *input;
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
    p0->position = v2(topleft.x, topleft.y);
    p0->color = color;

    Vertex *p1 = &vertex_buffer[vertex_count + 1];
    p1->position = v2(topleft.x + size.x, topleft.y);
    p1->color = color;

    Vertex *p2 = &vertex_buffer[vertex_count + 2];
    p2->position = v2(topleft.x, topleft.y + size.y);
    p2->color = color;

    Vertex *p3 = &vertex_buffer[vertex_count + 3];
    p3->position = v2(topleft.x + size.x, topleft.y + size.y);
    p3->color = color;

    u32 primitive = buffer->primitive_count;
    buffer->offsets[primitive] = vertex_count;
    buffer->counts[primitive] = 4;

    vertex_count += 4;
    buffer->primitive_count++;
}

inline Chunk *CurrentChunk()
{
    World *world = state->world;
    return world->chunks + (player->chunk_x + world->width * player->chunk_y);
}

// level stuff...

void LoadWorld()
{
    i32 width;
    i32 height;
    i32 channel = 3;

    state->world_memory.offset = 0;

    state->world = PushStructZero(&state->world_memory, World);
    World *world = state->world;
    world->width = 2;
    world->height = 2;

    world->chunks = PushArrayZero(&state->world_memory, Chunk, world->width * world->height);
    world->chunks[0].width = 16;
    world->chunks[0].height = 16;
}

void LoadState()
{
    player = &state->player;

    LoadWorld();
    *player = {};
    player->tile_x = 2;
    player->tile_y = 2;

    Chunk *chunk = CurrentChunk();
    chunk->enemy_count = 1;

    for (u32 i = 0; i < chunk->enemy_count; ++i)
    {
        chunk->enemies[i].position = v2(Round(Halton(i + 1, 2) * chunk->width), Round(Halton(i + 1, 3) * chunk->height));
    }
}

bool IsTileFree(i32 tile_x, i32 tile_y)
{
    Chunk *chunk = CurrentChunk();

    if (tile_x < 0 || tile_y < 0 || tile_x >= chunk->width || tile_y >= chunk->height)
    {
        return false;
    }

    for (u32 i = 0; i < chunk->tower_count; ++i)
    {
        Tower *tower = chunk->towers + i;
        if (tower->tile_x == tile_x && tower->tile_y == tile_y)
        {
            return false;
        }
    }

    return true;
}

void GameInitialize(u8 *memory, u64 memory_size)
{
    state = (GameState *) memory;
    *state = {};

    Arena *arena = (Arena *) memory;
    arena->memory = memory;
    arena->capacity = memory_size;
    arena->offset = sizeof(GameState);

    state->world_memory.capacity = KiloByte(10);
    state->world_memory.memory = PushBytes(arena, state->world_memory.capacity);

    LoadState();
}

RenderData *GameUpdate(GameInput *input_data, u8 *memory)
{
    input = input_data;
    state = (GameState *) memory;

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

    Chunk *chunk = CurrentChunk();

    // Begin Player stuff

    V2 player_local = v2(player->tile_x + player->offset.x, player->tile_y + player->offset.y);
    V2 player_move = {};

    if (KeyDown(Key_W))
    {
        player_move.y -= 1;
    }
    if (KeyDown(Key_S))
    {
        player_move.y += 1;
    }
    if (KeyDown(Key_A))
    {
        player_move.x -= 1;
    }
    if (KeyDown(Key_D))
    {
        player_move.x += 1;
    }
    
    player_move = Norm(player_move) * 5;
    player_local += input->delta * player_move;

    player->velocity = player->velocity + input->delta * ();

    i32 new_tile_x = Round(player_local.x);
    i32 new_tile_y = Round(player_local.y);

    // if (IsTileFree(new_tile_x, new_tile_y))
    // {
    player->tile_x = new_tile_x;
    player->tile_y = new_tile_y;
    player->offset.x = player_local.x - player->tile_x;
    player->offset.y = player_local.y - player->tile_y;
    // }
    // else
    // {
    //     player_local = player_start;
    // }

    // End player stuff

    if (KeyJustDown(Key_C))
    {
        assert(chunk->tower_count < lengthof(chunk->towers));
        Tower *tower = chunk->towers + chunk->tower_count++;
        tower->tile_x = player->tile_x;
        tower->tile_y = player->tile_y - 1;
    }

    for (u32 i = 0; i < chunk->enemy_count; ++i)
    {
        Enemy *enemy = chunk->enemies + i;
        V2 new_pos = enemy->position + Norm(player_local - enemy->position) * input->delta;
        i32 tile_x = Round(new_pos.x);
        i32 tile_y = Round(new_pos.y);
        if (IsTileFree(tile_x, tile_y))
        {
            enemy->position = new_pos;
        }
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

    for (u32 y = 0; y < 16; ++y)
    {
        for (u32 x = 0; x < 16; ++x)
        {
            DrawQuad(&level_buffer, v2(x * 32, y * 32), v2(32), v3(0.2, 0.8, 0.2));
        }
    }

    for (u32 i = 0; i < chunk->enemy_count; ++i)
    {
        Enemy *enemy = chunk->enemies + i;
        DrawQuad(&entity_buffer, enemy->position * 32, v2(32), v3(0.8, 0.2, 0.2));
    }

    for (u32 i = 0; i < chunk->tower_count; ++i)
    {
        Tower *tower = chunk->towers + i;
        DrawQuad(&entity_buffer, v2(tower->tile_x * 32, tower->tile_y * 32), v2(32), v3(0.2, 0.2, 0.8));
    }

    DrawQuad(&player_buffer, v2(player->tile_x, player->tile_y) * 32, v2(32), v3(0));
    DrawQuad(&player_buffer, player_local * 32, v2(32), v3(1));

    render->vertex_count = vertex_count;
    render->vertex_buffer = vertex_buffer;
    render->debug = BufferToDraw(&debug_buffer);
    render->level = BufferToDraw(&level_buffer);
    render->entities = BufferToDraw(&entity_buffer);
    render->player = BufferToDraw(&player_buffer);

    return render;
}
