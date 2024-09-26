#pragma once

#include "game_math.h"

void InitializeRenderer();
void ShutdownRenderer();
bool IsWindowOpen();
void DrawFrame();

bool IsKeyDown(char key);
bool IsKeyJustDown(char key);
