#pragma once

#include "game_math.h"

void InitializeRenderer();
void ShutdownRenderer();
bool IsWindowOpen();
void DrawFrame();

enum Key
{
    Key_W,
    Key_A,
    Key_S,
    Key_D,
    Key_C,
    Key_R,
    Key_Count,
};

bool IsKeyDown(Key key);
bool IsKeyJustDown(Key key);

f32 GetTime();
