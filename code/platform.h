#pragma once


#include "defines.h"
#include "memory.h"

struct FileRead
{
    u64 bytes_read;
    u8 *memory;
};

FileRead ReadFile(const char *filename, Arena *arena);
