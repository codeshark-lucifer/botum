#pragma once
#include "utils.h"

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
