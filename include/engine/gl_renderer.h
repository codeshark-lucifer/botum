#pragma once
#include "utils.h"
#include "shader.h"

#include <mathf/vectors.h>
#include <mathf/matries.h>
#include <mathf/mathf.h>
#include <mathf/quat.h>

#include <unordered_map>
#include <map>

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

enum Shape
{
    Rect,
    Cirle,
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
extern std::map<char, Character> Characters;

std::vector<Vertex> CreateQuad(vec2 pos, vec2 size, vec2 uv_offset, vec2 uv_size);
bool LoadFont(const std::string &filepath);

bool InitGLRender();
void glRender();
void DestroyGLContext();

u32 GetTexture(const str &path);
void DrawRectangle(vec2 pos, vec2 size, vec3 color = vec3(1.0f), Texture sprite = Texture{.path = ""});
void DrawTextUI(TextData text, vec2 pos, VerticalAlignment vAlign, TextAlignment hAlign);