#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <utils/macros.h>
#include <utils/vector.h>
#include <utils/quat.h>
#include <utils/matrix.h>

#include <new>       // for placement new
#include <utility>   // for std::forward
#include <vector>

// ============================
// Basic typedefs
// ============================
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef const char* str;

typedef IVec2 ivec2;
typedef Vec2 vec2;
typedef Vec3 vec3;
typedef Vec4 vec4;
typedef Mat3 mat3;
typedef Mat4 mat4;
typedef Quat quat;

template<typename T>
using Array = std::vector<T>; 

// ============================
// Logging & Assert Macro
// ============================
void print(str fmt, ...);

#define Assert(cond, fmt, ...)                                         \
    do {                                                                \
        if (!(cond)) {                                                  \
            print("Assert(＞﹏＜): %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__); \
            assert(cond);                                               \
        }                                                               \
    } while (0)

// ============================
// File read/write utilities
// ============================

// Reads entire file into a null-terminated string.
// Returns malloc'ed char* on success, nullptr on failure.
// The caller must free() the result.
char* read_file(str path);

// Writes buffer to file. Returns 1 on success, 0 on failure.
int write_file(str path, const char* buffer, u64 size);

// ============================
// Simple Bump Allocator
// ============================
struct BumpAllocator {
    size_t capacity;
    size_t used;
    char* memory;
};

// Create a bump allocator with `size` bytes
BumpAllocator MakeAllocator(size_t size);

// Allocate aligned memory from the bump allocator
void* BumpAllocAligned(BumpAllocator* alloc, size_t size, size_t align);

// Template helper for typed allocation
template <typename T, typename... Args>
T* BumpAlloc(BumpAllocator* alloc, Args&&... args)
{
    void* memory = BumpAllocAligned(alloc, sizeof(T), alignof(T));
    Assert(memory, "BumpAlloc out of memory");

    return new (memory) T(std::forward<Args>(args)...);
}
