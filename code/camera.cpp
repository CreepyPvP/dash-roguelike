#include "camera.h"

#include <math.h>


void InitializeCamera(Camera *camera, V3 pos, V3 front)
{
    camera->pos = pos;
    camera->front = Norm(front);

    // TODO: Calculate pitch and yaw here to prevent jumping
    camera->yaw = 0;
    camera->pitch = 0;
}

void UpdateCamera(Camera *camera)
{
    f32 delta = input->delta;
    f32 speed = 30;

    V2 movement = v2(0);

    if (KeyDown(Key_W)) {
        movement.x += 1;
    }
    if (KeyDown(Key_S)) {
        movement.x -= 1;
    }
    if (KeyDown(Key_A)) {
        movement.y -= 1;
    }
    if (KeyDown(Key_D)) {
        movement.y += 1;
    }

    movement = Norm(movement);

    V3 right = Norm(Cross(camera->front, v3(0, 1, 0)));

    camera->pos.x += camera->front.x * movement.x * delta * speed;
    camera->pos.y += camera->front.y * movement.x * delta * speed;
    camera->pos.z += camera->front.z * movement.x * delta * speed;
    camera->pos.x += right.x * movement.y * delta * speed;
    camera->pos.y += right.y * movement.y * delta * speed;
    camera->pos.z += right.z * movement.y * delta * speed;
};

void UpdateCameraMouse(Camera *camera)
{
    V2 mouse_input = input->mouse_pos - input->prev_mouse_pos;

    camera->yaw += mouse_input.x;
    camera->pitch -= mouse_input.y;

    if (camera->pitch > 89.0) {
        camera->pitch = 89.0;
    } else if (camera->pitch < -89.0) {
        camera->pitch = -89.0;
    }

    f32 pitch = camera->pitch;
    f32 yaw = camera->yaw;

    V3 dir;
    dir.x = cos(Radians(yaw)) * cos(Radians(pitch));
    dir.y = sin(Radians(pitch));
    dir.z = sin(Radians(yaw)) * cos(Radians(pitch));
    camera->front = Norm(dir);
}
