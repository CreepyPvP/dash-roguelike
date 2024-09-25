#pragma once

#include "game_math.h"

void renderer_Initialize();
void renderer_BeginFrame();
void renderer_EndFrame();
bool renderer_WindowOpen();
void renderer_Shutdown();

void renderer_DrawQuad(V2 botleft, V2 size, V3 color);

bool renderer_IsKeyDown(char key);
