#include <engine/utils.h>
#include <string.h>
#include <stdarg.h>

#include <engine/render_types.h>

RenderData *renderData = nullptr;

void print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

char *read_file(str path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return nullptr;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer)
    {
        fclose(f);
        return nullptr;
    }

    if (fread(buffer, 1, size, f) != (size_t)size)
    {
        free(buffer);
        fclose(f);
        return nullptr;
    }

    buffer[size] = '\0'; // null-terminate
    fclose(f);
    return buffer;
}

int write_file(str path, const char *buffer, u64 size)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return 0;

    if (fwrite(buffer, 1, size, f) != (size_t)size)
    {
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

BumpAllocator MakeAllocator(size_t size)
{
    BumpAllocator alloc{};
    alloc.memory = (char *)malloc(size);
    alloc.capacity = size;
    alloc.used = 0;
    return alloc;
}

inline size_t AlignForward(size_t ptr, size_t align)
{
    size_t mod = ptr & (align - 1);
    if (mod)
        ptr += (align - mod);
    return ptr;
}

void *BumpAllocAligned(BumpAllocator *alloc, size_t size, size_t align)
{
    size_t current = (size_t)alloc->memory + alloc->used;
    size_t aligned = AlignForward(current, align);
    size_t newUsed = aligned - (size_t)alloc->memory + size;

    if (newUsed > alloc->capacity)
        return nullptr;

    alloc->used = newUsed;
    return (void *)aligned;
}