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

    if (horizontal)
    {
        result.distance = Abs(walk_pos.x - position.x);
    }
    else
    {
        result.distance = Abs(walk_pos.y - position.y);
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

        if (IsKeyJustDown(Key_R))
        {
            *player = {};
            player->position = v2(2);
        }

        if (!game.player.flags & PLAYER_MOVING)
        {
            if (IsKeyJustDown(Key_W))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Up;
            }
            else if (IsKeyJustDown(Key_S))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Down;
            }
            else if (IsKeyJustDown(Key_A))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Left;
            }
            else if (IsKeyJustDown(Key_D))
            {
                player->flags |= PLAYER_MOVING;
                player->direction = Direction_Right;
            }
        }

        // static Direction direction;
        // if (IsKeyJustDown(Key_C))
        // {
        //     direction = (Direction) ((direction + 1) % 4);
        // }
        // ReadSensor(game.player.position + v2(0.5), direction);

        if (player->flags & PLAYER_MOVING)
        {
            SensorResult raycast = ReadSensor(player->position + player_sensor_offset[player->direction], player->direction);

            f32 move_dist = delta * 2;
            if (move_dist >= raycast.distance)
            {
                move_dist = raycast.distance;
                player->flags &= ~PLAYER_MOVING;
            }

	        player->position += direction_to_vec[player->direction] * move_dist;
        }

        DrawFrame();
    }

    ShutdownRenderer();
}
