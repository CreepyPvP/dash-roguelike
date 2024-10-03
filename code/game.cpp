#include <stdio.h>

#include "defines.h"
#include "game.h"
#include "memory.h"
#include "game_math.h"
#include "platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// NOTE: Cpp context causes name mangling. sad :(
extern "C"
{
    __declspec(dllexport) RenderData * __stdcall GameUpdate(GameInput *input_data, u8 *memory);
    __declspec(dllexport) void _stdcall GameInitialize(u8 *memory, u64 memory_size);
}

Player *player;
Level *level;

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

// Level stuff...

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

struct SensorResult
{
    u32 status;
    f32 distance;
    V2 hit;
};

SensorResult ReadSensor(V2 position, Direction direction)
{
    SensorResult result = {};

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
        if (grid_x < 0 || grid_x >= level->width || grid_y < 0 || grid_y >= level->height)
        {
            result.hit = walk_pos;
            result.status = 2;
            break;
        }

        if (level->tiles[grid_x + grid_y * level->width])
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

char *level_files[] = {
    "assets/level_0.png",
};

void LoadLevel(u32 stage)
{
    i32 width;
    i32 height; 
    i32 channel = 3;

    u8 *data = stbi_load(level_files[stage], &width, &height, &channel, STBI_rgb);
    assert(data);

    Level level = {};
    level.width = width;
    level.height = height;
    assert(width * height < sizeof(level.tiles));

    Player player = {};

    u8 *walk = data;

    for (i32 y = 0; y < height; ++y)
    {
        for (i32 x = 0; x < width; ++x)
        {
            if (walk[0] == 0 && walk[1] == 0 && walk[2] == 0)
            {
                level.tiles[x + y * width] = 1;
            }

            if (walk[0] == 0 && walk[1] == 0 && walk[2] == 255)
            {
                player.position = v2(x, y);
            }

            if (walk[0] == 255 && walk[1] == 0 && walk[2] == 0)
            {
                assert(level.enemy_count < lengthof(level.enemies));
                Enemy *enemy = &level.enemies[level.enemy_count++];
                enemy->position = v2(x, y);
            }

            walk += 3;
        }
    }

    stbi_image_free(data);

    state->player = player;
    state->level = level;
}

void GameInitialize(u8 *memory, u64 memory_size)
{
    state = (GameState *) memory;
    *state = {};

    Arena *arena = (Arena *) memory;
    arena->memory = memory;
    arena->capacity = memory_size;
    arena->offset = sizeof(GameState);

    LoadLevel(0);
}

RenderData *GameUpdate(GameInput *input_data, u8 *memory)
{
    input = input_data;
    state = (GameState *) memory;

    player = &state->player;
    level = &state->level;

    vertex_count = {};
    level_buffer = {};
    debug_buffer = {};
    enemy_buffer = {};

    if (KeyJustDown(Key_R))
    {
        LoadLevel(0);
    }

    if (!player->flags & PLAYER_MOVING)
    {
        if (KeyJustDown(Key_W))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Up;
        }
        else if (KeyJustDown(Key_S))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Down;
        }
        else if (KeyJustDown(Key_A))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Left;
        }
        else if (KeyJustDown(Key_D))
        {
            player->flags |= PLAYER_MOVING;
            player->direction = Direction_Right;
        }
    }

    if (player->flags & PLAYER_MOVING)
    {
        SensorResult raycast = ReadSensor(player->position + player_sensor_offset[player->direction], player->direction);

        f32 move_dist = input->delta * 20;
        if (move_dist >= raycast.distance)
        {
            move_dist = raycast.distance;
            player->flags &= ~PLAYER_MOVING;
        }

        player->position += direction_to_vec[player->direction] * move_dist;
    }

    for (u32 i = 0; i < level->enemy_count; ++i)
    {
        Enemy *enemy = &level->enemies[i];
        if (AABBCollision(player->position, player->position + v2(1), enemy->position, enemy->position + v2(1)))
        {
            enemy->flags |= ENEMY_DEAD;
        }
    }

    // We render at 960 x 540
    // 0,0 ------------> 960,0
    // \
    // \ each tile is 32 x 32
    // \ 
    // \
    // 0,540

    for (u32 y = 0; y < level->height; ++y)
    {
        for (u32 x = 0; x < level->width; ++x)
        {
            if (level->tiles[x + y * level->width])
            {
                assert(vertex_count + 4 <= lengthof(vertex_buffer));

                V3 color = v3(0.6);

                Vertex *p0 = &vertex_buffer[vertex_count];
                p0->position = v2(x * 32, y * 32);
                p0->color = color;

                Vertex *p1 = &vertex_buffer[vertex_count + 1];
                p1->position = v2(x * 32 + 32, y * 32);
                p1->color = color;

                Vertex *p2 = &vertex_buffer[vertex_count + 2];
                p2->position = v2(x * 32, y * 32 + 32);
                p2->color = color;

                Vertex *p3 = &vertex_buffer[vertex_count + 3];
                p3->position = v2(x * 32 + 32, y * 32 + 32);
                p3->color = color;

                assert(level_buffer.primitive_count < lengthof(level_buffer.offsets));
                u32 primitive = level_buffer.primitive_count;
                level_buffer.offsets[primitive] = vertex_count;
                level_buffer.counts[primitive] = 4;

                vertex_count += 4;
                level_buffer.primitive_count++;

                // DrawQuad(x * 32, y * 32, 32, 32, v3(0.6));
            }
        }
    }

    RenderData *render = &state->render_data;
    render->vertex_count = vertex_count;
    render->vertex_buffer = vertex_buffer;
    render->debug = BufferToDraw(&debug_buffer);
    render->level = BufferToDraw(&level_buffer);
    render->enemies = BufferToDraw(&enemy_buffer);

    return render;
}
