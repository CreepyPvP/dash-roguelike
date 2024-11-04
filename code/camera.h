#pragma once

#include "game.h"
#include "game_math.h"


struct Camera
{
    V3 pos;
    V3 front;

    f32 yaw;
    f32 pitch;
};

void InitializeCamera(Camera *camera, V3 pos, V3 front);
void UpdateCamera(Camera *camera);
void UpdateCameraMouse(Camera *camera);
