#pragma once
#include <engine/gl_renderer.hpp>

inline void DrawUIText(const std::string &text, vec2 pos, float scale, vec3 color = vec3(1.0f), float layer = 0.0f)
{
    float x = pos.x;
    float y = pos.y;

    // Treat `scale` as the desired pixel size for text. Convert to a
    // multiplicative scale factor relative to the loaded font pixel size.
    float scaleFactor = 1.0f;
    if (font.pixelSize > 0)
        scaleFactor = scale / (float)font.pixelSize;

    // Compute baseline assuming `pos.y` is the top of the line.
    float baseline = y;
    if (font.ascender > 0)
        baseline = y + (font.ascender * scaleFactor);

    for (char c : text)
    {
        if (font.glyphs.find(c) == font.glyphs.end())
            continue;

        Glyph &g = font.glyphs[c];

        float xpos = x + g.bearing.x * scaleFactor;
        float ypos = baseline - (g.bearing.y * scaleFactor);

        vec2 size = vec2(g.size.x, g.size.y) * scaleFactor;

        Transform trans = {};
        trans.ioffset = g.offset;
        trans.isize = g.size;
        trans.pos = vec2(xpos, ypos);
        trans.size = size;
        trans.color = vec4(color, 1.0f);
        trans.layer = layer;
        renderData->uiTransforms.push_back(trans);

        x += (g.advance >> 6) * scaleFactor;
    }
}