#pragma once
#include <cstdint>

#include <string>
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef char32_t c32;

typedef std::string str;

template<typename T>
using array = std::vector<T>;


char *read_file(str path);
int write_file(str path, const char *buffer, u64 size);