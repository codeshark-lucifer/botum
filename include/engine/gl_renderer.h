#pragma once
#include "utils.h"
#include "shader.h"

#include <mathf/vectors.h>
#include <mathf/matries.h>
#include <mathf/mathf.h>
#include <mathf/quat.h>

#include <unordered_map>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

struct Vertex
{
    vec2 position;
    vec2 uv;
};

struct Character
{
    u32 texID;
    ivec2 Size;
    ivec2 Bearing;
    u32 Advance;
};

enum class Shape
{
    Rect,
    Circle,
    Triangle
};

struct Texture 
{
    str path;
    u32 id;
};

struct Batch
{
    // contian info for render
    std::vector<Vertex> vertices;
    vec3 color;
    Shape shape;
    Texture sprite;
};

struct GLContext
{
    u32 vao, vbo;
    std::vector<Batch> world_batch;        // World batch
    std::vector<Batch> ui_batch;           // UI batch
    std::unordered_map<str, u32> textures; // container map of text
    Shader *shader;
    FT_Face fontFace;
    hb_font_t *hbFont;
};

struct TextData
{
    str content;
    float scale;
    vec3 color;
    float maxWidth;
};

enum class TextAlignment
{
    Left,
    Center,
    Right
};

enum class VerticalAlignment
{
    Top,
    Middle,
    Bottom
};

// --- Globals ---
extern GLContext gl;
extern std::map<u32, Character> GlyphCache;

std::vector<Vertex> CreateQuad(vec2 pos, vec2 size, vec2 uv_offset, vec2 uv_size);
bool LoadFont(const std::string &filepath);

bool InitGLRender();
void glRender();
void DestroyGLContext();

u32 GetTexture(const str &path);
void DrawRectangle(vec2 pos, vec2 size, vec3 color = vec3(1.0f), Texture sprite = Texture{.path = ""});
void DrawTextUI(TextData text, vec2 pos, VerticalAlignment vAlign, TextAlignment hAlign);