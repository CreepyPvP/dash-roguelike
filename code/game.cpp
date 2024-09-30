#include <stdio.h>

#include "defines.h"
#include "game.h"
#include "memory.h"
#include "game_math.h"
#include "renderer.h"
#include "platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Player player;
Level level;

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
        if (grid_x < 0 || grid_x >= level.width || grid_y < 0 || grid_y >= level.height)
        {
            result.hit = walk_pos;
            result.status = 2;
            break;
        }

        if (level.tiles[grid_x + grid_y * level.width])
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

    level = {};
    level.width = width;
    level.height = height;
    assert(width * height < sizeof(level.tiles));

    player = {};

    u8 *walk = data;

    // flip y or madness ensues
    for (i32 y = height - 1; y >= 0; --y)
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
}

void InitializeGame()
{
    memory_Initialize();
    LoadLevel(0);
}

void UpdateGame(f32 delta)
{
    debug_ray_count = 0;

    if (IsKeyJustDown(Key_R))
    {
        LoadLevel(0);
    }

    if (!player.flags & PLAYER_MOVING)
    {
        if (IsKeyJustDown(Key_W))
        {
            player.flags |= PLAYER_MOVING;
            player.direction = Direction_Up;
        }
        else if (IsKeyJustDown(Key_S))
        {
            player.flags |= PLAYER_MOVING;
            player.direction = Direction_Down;
        }
        else if (IsKeyJustDown(Key_A))
        {
            player.flags |= PLAYER_MOVING;
            player.direction = Direction_Left;
        }
        else if (IsKeyJustDown(Key_D))
        {
            player.flags |= PLAYER_MOVING;
            player.direction = Direction_Right;
        }
    }

    // static Direction direction;
    // if (IsKeyJustDown(Key_C))
    // {
    //     direction = (Direction) ((direction + 1) % 4);
    // }
    // ReadSensor(player.position + v2(0.5), direction);

    if (player.flags & PLAYER_MOVING)
    {
        SensorResult raycast = ReadSensor(player.position + player_sensor_offset[player.direction], player.direction);

        f32 move_dist = delta * 20;
        if (move_dist >= raycast.distance)
        {
            move_dist = raycast.distance;
            player.flags &= ~PLAYER_MOVING;
        }

        player.position += direction_to_vec[player.direction] * move_dist;
    }

    for (u32 i = 0; i < level.enemy_count; ++i)
    {
        Enemy *enemy = &level.enemies[i];
        if (AABBCollision(player.position, player.position + v2(1), enemy->position, enemy->position + v2(1)))
        {
            enemy->flags |= ENEMY_DEAD;
        }
    }
}
