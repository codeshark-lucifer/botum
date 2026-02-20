#pragma once

#include <engine/shader.h>
#include <stb/stb_image.h>
#include <engine/render_types.h>
#include <platform/win32.h>
#include <string>
#include <map>

constexpr str FONT_FILE_PATH     = "assets/fonts/arial.ttf";
constexpr str SHADER_SCENE_VERT  = "assets/shaders/scene.vert";
constexpr str SHADER_SCENE_FRAG  = "assets/shaders/scene.frag";
constexpr str ATLAS_TEXTURE_PATH = "assets/sprites/sample.png";

struct GLContext
{
    u32 shader;
    u32 vao;
    u32 transSSBO;

    u32 atlasTex;
    ivec2 texSize;
};

struct Font
{
    std::map<char, Glyph> glyphs;
    u32 texture;
    int atlasWidth;
    int atlasHeight;
    int pixelSize;
    int ascender; // in pixels (face->size->metrics.ascender >> 6)
};

extern GLContext gl;
extern Font font;

bool InitGLRenderer();
void glRender();
void DestroyGLContext();