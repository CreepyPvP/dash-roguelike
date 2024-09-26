#include <stdio.h>

#include "defines.h"
#include "memory.h"
#include "game_math.h"
#include "renderer.h"
#include "game.h"

Game game;

V2 direction_to_vec[4] = {
    v2(0, 1),
    v2(-1, 0),
    v2(0, -1),
    v2(1, 0),
};

struct SensorResult
{
    u32 status;
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

    while (true)
    {
        if (grid_x < 0 || grid_x >= game.level.width || grid_y < 0 || grid_y >= game.level.height)
        {
            result.hit = walk_pos;
            result.status = 2;
            break;
        }

        if (game.level.tiles[grid_x + grid_y * game.level.width])
        {
            result.hit = walk_pos;
            result.status = 1;
            break;
        }

        grid_x += dir.x;
        grid_y += dir.y;
        walk_pos += dir;
    }

    DebugRay *ray = AllocDebugRay();
    ray->p0 = position;
    ray->p1 = result.hit;

    return result;
}

i32 main()
{
    memory_Initialize();
    InitializeRenderer();

    game = {};
    game.level.width = 16;
    game.level.height = 16;
    game.player.position.x = 2;
    game.player.position.y = 2;

    for (u32 x = 0; x < game.level.width; ++x)
    {
        for (u32 y = 0; y < game.level.height; ++y)
        {
            u8 tile = 0;

            if (x == 0 || x == (game.level.width - 1) || y == 0 || y == (game.level.height - 1))
            {
                tile = 1;
            }

            if (x == 2 && y == 4)
            {
                tile = 1;
            }

            game.level.tiles[x + y * game.level.width] = tile;
        }
    }

    Player *player = &game.player;

    while (IsWindowOpen())
    {
        f32 delta = 1.0 / 60.0;

        debug_ray_count = 0;
         
        if (IsKeyDown('R'))
        {
            *player = {};
            player->position = v2(2);
        }

        if (!game.player.flags & PLAYER_MOVING)
        {
            if (IsKeyDown('W'))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Up;
            }
            else if (IsKeyDown('S'))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Down;
            }
            else if (IsKeyDown('A'))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Left;
            }
            else if (IsKeyDown('D'))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Right;
            }
        }

        static Direction direction;
        if (IsKeyJustDown('C'))
        {
            direction = (Direction) ((direction + 1) % 4);
        }
        ReadSensor(game.player.position + v2(0.5), direction);

        if (player->flags & PLAYER_MOVING && !IsKeyDown(' '))
        {
            player->position += direction_to_vec[player->direction] * delta;
        }

        DrawFrame();
    }

    ShutdownRenderer();
}
