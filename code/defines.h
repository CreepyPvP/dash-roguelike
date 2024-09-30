#pragma once

#include <stdint.h>

#define DEBUG

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define KiloByte(amount) (amount * 1024)
#define MegaByte(amount) (KiloByte(amount) * 1024)

#define lengthof(x) (sizeof(x) / sizeof(x[0]))

#ifdef PLATFORM
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
