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
    i32 offsets[128];
    i32 counts[128];
};

u32 vertex_count;
Vertex vertex_buffer[1024];

MultiDrawBuffer level_buffer;
MultiDrawBuffer debug_buffer;
MultiDrawBuffer enemy_buffer;

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

SingleDraw DrawQuad(V2 topleft, V2 size, V3 color)
{
    assert(vertex_count + 4 <= lengthof(vertex_buffer));

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

    SingleDraw draw = {};
    draw.offset = vertex_count;
    draw.count = 4;

    vertex_count += 4;
    return draw;
}

// World stuff...

V2 direction_to_vec[4] = {
    v2(0, 1),
    v2(-1, 0),
    v2(0, -1),
    v2(1, 0),
};

V2 player_sensor_offset[4] = {
    v2(0.5, 1),
    v2(0, 0.5),
    v2(0.5, 0),
    v2(1, 0.5),
};

inline Chunk *CurrentChunk()
{
    World *world = state->world;
    return world->chunks + (player->chunk_x + world->width * player->chunk_y);
}

struct SensorResult
{
    u32 status;
    f32 distance;
    V2 hit;
};

SensorResult ReadSensor(V2 position, Direction direction)
{
    SensorResult result = {};

    Chunk *chunk = CurrentChunk();

    bool horizontal = direction & 1;
    V2 dir = direction_to_vec[direction];
    i32 grid_x = position.x;
    i32 grid_y = position.y;

    V2 walk_pos = position;

    switch (direction)
    {
        case Direction_Up: { 
            walk_pos.y = Floor(walk_pos.y);
            break;
        }
        case Direction_Down: { 
            walk_pos.y = Floor(walk_pos.y) + 1;
            break;
        }
        case Direction_Left: { 
            walk_pos.x = Floor(walk_pos.x) + 1;
            break;
        }
        case Direction_Right: { 
            walk_pos.x = Floor(walk_pos.x);
            break;
        }
    }

    while (true)
    {
        if (grid_x < 0 || grid_x >= 16 || grid_y < 0 || grid_y >= 16)
        {
            result.hit = walk_pos;
            result.status = 2;
            break;
        }

        if (chunk->tiles[grid_x + grid_y * 16])
        {
            result.hit = walk_pos;
            result.status = 1;
            break;
        }

        grid_x += dir.x;
        grid_y += dir.y;
        walk_pos += dir;
    }

    if (horizontal)
    {
        result.distance = Abs(walk_pos.x - position.x);
    }
    else
    {
        result.distance = Abs(walk_pos.y - position.y);
    }

    // DebugRay *ray = AllocDebugRay();
    // ray->p0 = position;
    // ray->p1 = result.hit;

    return result;
}

// level stuff...

const char *level_files[] = {
    "assets/level_0.png",
};

void LoadWorld()
{
    i32 width;
    i32 height; 
    i32 channel = 3;

    // u8 *data = stbi_load(level_files[stage], &width, &height, &channel, STBI_rgb);
    // assert(data);

    state->world_memory.offset = 0;

    state->world = PushStruct(state->world_memory, World);
    World *world = state->world;
    world->width = 2;
    world->height = 2;

    world->chunks = PushArray(state->world_memory, Chunk, world->width * world->height);
    world->chunks[0].width = 16;
    world->chunks[0].height = 16;
    world->chunks[0].tiles = PushBytes(state->world_memory, 256);

    u8 tiles00[16][16] = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, },
    };

    memcpy(world->chunks[0].tiles, tiles00, 256);

    Player player = {};

    // u8 *walk = data;
    //
    // for (i32 y = 0; y < height; ++y)
    // {
    //     for (i32 x = 0; x < width; ++x)
    //     {
    //         if (walk[0] == 0 && walk[1] == 0 && walk[2] == 0)
    //         {
    //             world.tiles[x + y * width] = 1;
    //         }
    //
    //         if (walk[0] == 0 && walk[1] == 0 && walk[2] == 255)
    //         {
    //             player.position = v2(x, y);
    //         }
    //
    //         if (walk[0] == 255 && walk[1] == 0 && walk[2] == 0)
    //         {
    //             assert(world.enemy_count < lengthof(world.enemies));
    //             Enemy *enemy = world.enemies + world.enemy_count++;
    //             enemy->position = v2(x, y);
    //         }
    //
    //         walk += 3;
    //     }
    // }

    // stbi_image_free(data);

    state->player = player;
    state->world = world;
}

void GameInitialize(u8 *memory, u64 memory_size)
{
    state = (GameState *) memory;
    *state = {};

    Arena *arena = (Arena *) memory;
    arena->memory = memory;
    arena->capacity = memory_size;
    arena->offset = sizeof(GameState);

    state->world_memory.capacity = KiloBytes(10);
    state->world_memory.memory = PushArena(arena, state->world_memory.capacity);

    LoadWorld();
}

RenderData *GameUpdate(GameInput *input_data, u8 *memory)
{
    input = input_data;
    state = (GameState *) memory;
    printf("%f\n", input->delta);

    player = &state->player;
    world = &state->world;

    vertex_count = {};
    level_buffer = {};
    debug_buffer = {};
    enemy_buffer = {};

    if (KeyJustDown(Key_R))
    {
        LoadWorld();
    }

    if (!(player->flags & PLAYER_MOVING))
    {
        if (KeyDown(Key_W))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Down;
        }
        else if (KeyDown(Key_S))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Up;
        }
        else if (KeyDown(Key_A))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Left;
        }
        else if (KeyDown(Key_D))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Right;
        }
    }

    if (player->flags & PLAYER_MOVING)
    {
        SensorResult raycast = ReadSensor(player->position + player_sensor_offset[player->direction], player->direction);

        f32 move_dist = input->delta * 50;
        if (move_dist >= raycast.distance)
        {
            move_dist = raycast.distance;
            player->flags &= ~PLAYER_MOVING;
        }

        player->position += direction_to_vec[player->direction] * move_dist;
    }

    Chunk *chunk = CurrentChunk();

    for (u32 i = 0; i < chunk->enemy_count; ++i)
    {
        Enemy *enemy = chunk->enemies + i;
        if (AABBCollision(player->position, player->position + v2(1), enemy->position, enemy->position + v2(1)))
        {
            enemy->flags |= ENEMY_DEAD;
        }
    }

    // We render at 960 x 540
    // 0,0 ------------> 960,0
    // |
    // | each tile is 32 x 32
    // | 
    // |
    // 0,540

    RenderData *render = &state->render_data;

    for (u32 y = 0; y < 16; ++y)
    {
        for (u32 x = 0; x < 16; ++x)
        {
            if (chunk->tiles[x + y * 16])
            {
                DrawQuad(&level_buffer, v2(x * 32, y * 32), v2(32), v3(0, 0, 1));
            }
        }
    }

    for (u32 i = 0; i < chunk->enemy_count; ++i)
    {
        Enemy *enemy = chunk->enemies + i;

        if (enemy->flags & ENEMY_DEAD)
        {
            continue;
        }

        DrawQuad(&enemy_buffer, enemy->position * 32, v2(32), v3(1, 0, 0));
    }

    render->player = DrawQuad(player->position * 32, v2(32), v3(0, 1, 0));

    render->vertex_count = vertex_count;
    render->vertex_buffer = vertex_buffer;
    render->debug = BufferToDraw(&debug_buffer);
    render->level = BufferToDraw(&level_buffer);
    render->enemies = BufferToDraw(&enemy_buffer);

    return render;
}
