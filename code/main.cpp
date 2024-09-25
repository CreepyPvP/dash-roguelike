#include <stdio.h>

#include "defines.h"
#include "memory.h"
#include "game_math.h"
#include "renderer.h"

#define PLAYER_MOVING (1 << 0)

enum Direction
{
    Direction_Up,
    Direction_Left,
    Direction_Down,
    Direction_Right,
};

V2 direction_to_vec[4] = {
    v2(0, 1),
    v2(-1, 0),
    v2(0, -1),
    v2(1, 0),
};

struct Player
{
    u32 flags;
    Direction direction;
    V2 position;
};

struct Level
{
    u32 width;
    u32 height;
    u8 tiles[256];
};

struct SensorResult
{
    u32 status;
    V2 hit;
};

SensorResult CheckSensor(V2 position, Direction direction, Level *level)
{
    SensorResult result = {};

    bool horizontal = direction & 1;
    V2 dir = direction_to_vec[direction];
    i32 grid_x = position.x;
    i32 grid_y = position.y;

    V2 walk_pos = position;

    while (true)
    {
        if (grid_x < 0 || grid_x >= level->width || grid_y < 0 || grid_y >= level->height)
        {
            result.hit = walk_pos;
            result.status = 2;
            return result;
        }

        if (level->tiles[grid_x + grid_y * level->width])
        {
            result.hit = walk_pos;
            result.status = 1;
            return result;
        }

        grid_x += dir.x;
        grid_y += dir.y;
        walk_pos += dir;
    }

    f32 line_width = 4;
    if (horizontal)
    {
        f32 start = Min(walk_pos.x, position.x);
        f32 end = Max(walk_pos.x, position.x);
        renderer_DrawQuad(v2(start, position.y - line_width / 2), v2(end - start, line_width), v3(0, 1, 0));
    }
    else
    {
        f32 start = Min(walk_pos.y, position.y);
        f32 end = Max(walk_pos.y, position.y);
        renderer_DrawQuad(v2(position.x - line_width / 2, start), v2(line_width, end - start), v3(0, 1, 0));
    }

    return result;
}

i32 main()
{
    memory_Initialize();
    renderer_Initialize();

    Level level = {};
    level.width = 16;
    level.height = 16;

    Player player = {};
    player.position.x = 2;
    player.position.y = 2;

    for (u32 x = 0; x < level.width; ++x)
    {
        for (u32 y = 0; y < level.height; ++y)
        {
            u8 tile = 0;

            if (x == 0 || x == (level.width - 1) || y == 0 || y == (level.height - 1))
            {
                tile = 1;
            }

            if (x == 2 && y == 4)
            {
                tile = 1;
            }

            level.tiles[x + y * level.width] = tile;
        }
    }

    f32 tilesize = 32;

    while (renderer_WindowOpen())
    {
        f32 delta = 1.0 / 60.0;

        renderer_BeginFrame();
         
        if (renderer_IsKeyDown('R'))
        {
            player = {};
            player.position.x = 2;
            player.position.y = 2;
        }

        if (!player.flags & PLAYER_MOVING)
        {
            if (renderer_IsKeyDown('W'))
            {
                player.flags = player.flags | PLAYER_MOVING;
                player.direction = Direction_Up;
            }
            else if (renderer_IsKeyDown('S'))
            {
                player.flags = player.flags | PLAYER_MOVING;
                player.direction = Direction_Down;
            }
            else if (renderer_IsKeyDown('A'))
            {
                player.flags = player.flags | PLAYER_MOVING;
                player.direction = Direction_Left;
            }
            else if (renderer_IsKeyDown('D'))
            {
                player.flags = player.flags | PLAYER_MOVING;
                player.direction = Direction_Right;
            }
        }

        CheckSensor(player.position, Direction_Right, &level);

        if (player.flags & PLAYER_MOVING && !renderer_IsKeyDown(' '))
        {
            player.position += direction_to_vec[player.direction] * delta;
        }

        for (u32 x = 0; x < level.width; ++x)
        {
            for (u32 y = 0; y < level.width; ++y)
            {
                if (level.tiles[x + y * level.width])
                {
                    renderer_DrawQuad(v2(x * tilesize , y * tilesize), v2(tilesize), v3(0.2, 0.2, 0.9));
                }
            }
        }

        renderer_DrawQuad(v2(player.position.x * tilesize, player.position.y * tilesize), v2(tilesize), v3(0.9, 0.2, 0.2));

        renderer_EndFrame();
    }

    renderer_Shutdown();
}
