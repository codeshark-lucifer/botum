#pragma once
#include "utils.h"

struct alignas(16) Transform
{
    ivec2 ioffset;     // 8 bytes
    ivec2 isize;       // 8 bytes
    vec2 pos;          // 8 bytes
    vec2 size;         // 8 bytes
    vec4 color;        // 16 bytes
    int renderOptions; // 4 bytes
    float layer;       // 4 bytes
    int _padding[2];   // padding for std430 alignment
};

struct Camera2D
{
    vec2 pos;
    vec2 dimensions;  // logical world size
    vec2 framebuffer; // actual pixel framebuffer

    mat4 matrix()
    {
        return Mat4::Ortho(0.0f, dimensions.x, dimensions.y, 0.0f, -1.0f, 1.0f);
    }
};

struct RenderData
{
    Camera2D camera;
    Array<Transform> transforms;
    Array<Transform> uiTransforms;

    void OnResize(int x, int y)
    {
        // If the logical camera has not been initialized (zero), use the
        // first resize (typically window creation) to set the logical view
        // size. On subsequent resizes we only update the framebuffer size so
        // the logical camera remains fixed and content scales with the
        // viewport.
        if (camera.dimensions.x == 0.0f && camera.dimensions.y == 0.0f)
        {
            camera.dimensions = {(float)x, (float)y};
        }
        camera.framebuffer = {(float)x, (float)y};
    }
};

struct Glyph
{
    ivec2 size;    // glyph bitmap size
    ivec2 bearing; // offset from baseline
    u32 advance;   // advance.x from FreeType
    ivec2 offset;  // atlas position (ioffset)
};

extern RenderData *renderData;
