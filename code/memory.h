#pragma once

#include "defines.h"

struct Arena
{
    u8 *memory;
    u64 offset;
    u64 capacity;
};

struct TempMemory
{
    Arena *arena;
    u64 offset;
};

#define PushStruct(arena, type) ((type *) AllocateBytes(arena, sizeof(type), alignof(type)))
#define PushBytes(arena, size) ((u8 *) AllocateBytes(arena, size, alignof(u8 *)))
#define PushArray(arena, type, size) ((type *) AllocateBytes(arena, sizeof(type) * size, alignof(type)))

TempMemory BeginTempRegion(Arena *arena);
void EndTempRegion(TempMemory region);
u8 *AllocateBytes(Arena *arena, u64 size, u64 align);

TempMemory ScratchAllocate();

extern Arena scratch;
