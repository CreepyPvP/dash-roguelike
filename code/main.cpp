#include <stdio.h>

#include "defines.h"
#include "memory.h"
#include "game_math.h"
#include "renderer.h"

struct Game
{
    u32 width;
    u32 height;
    u8 tiles[256];
};

int main()
{
    memory_Initialize();
    renderer_Initialize();

    Game game = {};
    game.width = 16;
    game.height = 16;

    for (u32 x = 0; x < game.width; ++x)
    {
        for (u32 y = 0; y < game.height; ++y)
        {
            u8 tile = 0;

            if (x == 0 || x == (game.width - 1) || y == 0 || y == (game.height - 1))
            {
                tile = 1;
            }

            if (x == 2 && y == 4)
            {
                tile = 1;
            }

            game.tiles[x + y * game.width] = tile;
        }
    }

    f32 tilesize = 32;

    while (renderer_WindowOpen())
    {
        renderer_BeginFrame();

        for (u32 x = 0; x < game.width; ++x)
        {
            for (u32 y = 0; y < game.width; ++y)
            {
                if (game.tiles[x + y * game.width])
                {
                    renderer_DrawQuad(v2(x * tilesize , y * tilesize), v2(tilesize), v3(0.2, 0.2, 0.9));
                }
            }
        }
        renderer_EndFrame();
    }

    renderer_Shutdown();
}
