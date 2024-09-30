#pragma once

#include "memory.h"

struct FileRead
{
    u64 bytes_read;
    u8 *memory;
};

FileRead ReadFile(const char *filename, Arena *arena);

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
