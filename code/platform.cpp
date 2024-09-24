#include "platform.h"

#include <assert.h>
#include <stdio.h>

FileRead ReadFile(const char *filename, Arena *arena)
{
    FileRead read = {};

    FILE *file = fopen(filename, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    u8 *buffer = PushBytes(arena, len + 1);
    fseek(file, 0, SEEK_SET);
    fread(buffer, len, 1, file);
    buffer[len] = 0;

    fclose(file);

    read.bytes_read = len;
    read.memory = buffer;

    return read;
}
