#include <engine/utils.h>
#include <cstdio>
#include <cstdlib>

char *read_file(str path)
{
    FILE *f = fopen(path.c_str(), "rb");
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
    FILE *f = fopen(path.c_str(), "wb");
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