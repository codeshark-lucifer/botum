#include <platform/win32.h>
#include <glad/glad.h>
#include <engine/shader.h>
#include <mathf/matries.h>
#include <stb/stb_image.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

// --- Structures ---
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

struct Sprite
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
    Sprite sprite;
};

struct GLContext
{
    u32 vao, vbo;
    std::vector<Batch> world_batch;        // World batch
    std::vector<Batch> ui_batch;           // UI batch
    std::unordered_map<str, u32> textures; // container map of text
    Shader *shader;
};

struct Text
{
    str content;
    float scale;
    vec3 color;
};

// --- Globals ---
static GLContext gl;
std::map<char, Character> Characters;

// --- Helper: Create Quad Vertices ---
std::vector<Vertex> CreateQuad(vec2 pos, vec2 size, vec2 uv_offset, vec2 uv_size)
{
    return {
        {{pos.x, pos.y}, {uv_offset.x, uv_offset.y}},
        {{pos.x, pos.y + size.y}, {uv_offset.x, uv_offset.y + uv_size.y}},
        {{pos.x + size.x, pos.y + size.y}, {uv_offset.x + uv_size.x, uv_offset.y + uv_size.y}},

        {{pos.x + size.x, pos.y + size.y}, {uv_offset.x + uv_size.x, uv_offset.y + uv_size.y}},
        {{pos.x + size.x, pos.y}, {uv_offset.x + uv_size.x, uv_offset.y}},
        {{pos.x, pos.y}, {uv_offset.x, uv_offset.y}}};
}

// --- Font Loading ---
bool LoadFont(const std::string &filepath)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        return false;

    FT_Face face;
    if (FT_New_Face(ft, filepath.c_str(), 0, &face))
        return false;

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        u32 texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width, face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (u32)face->glyph->advance.x};
        Characters[c] = character;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return true;
}

void Start();
void Update(float deltaTime);

void Render();
int main()
{
    if (!InitPlatform())
        return -1;
    if (!CreateWindowPlatform("Botum Engine", 956, 540))
        return -1;

    if (!LoadFont("assets/fonts/arial.ttf"))
    {
        printf("Warning: Failed to load font\n");
    }

    Shader shader("assets/shaders/scene.vert", "assets/shaders/scene.frag");
    gl.shader = &shader;

    // --- Buffer Setup ---
    glGenVertexArrays(1, &gl.vao);
    glGenBuffers(1, &gl.vbo);
    glBindVertexArray(gl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
    // Reserve space for 6 vertices (one quad)
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)sizeof(vec2));

    Start();
    while (!ShouldClose())
    {
        Event event;
        PollEvent(&event);
        glViewport(0, 0, input.screen.x, input.screen.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader.Use();
        mat4 worldProj = mat4::Ortho(0, (float)input.screen.x, 0, (float)input.screen.y, -1.0f, 1.0f);
        shader.SetUniform("projection", worldProj);
        Update(event.deltaTime);
        Render();
        gl.ui_batch.clear();
        gl.world_batch.clear();
        SwapBuffersWindow();
    }
    glDeleteBuffers(1, &gl.vbo);
    glDeleteVertexArrays(1, &gl.vao);
    DestroyPlatform();
    return 0;
}

u32 GetTexture(const str &path)
{
    if (path.empty())
        return 0;

    // Check if already in cache
    auto it = gl.textures.find(path);
    if (it != gl.textures.end())
        return it->second;

    // Not in cache, load it
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    u32 textureID = 0;
    if (data)
    {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        gl.textures[path] = textureID; // Cache it
    }
    return textureID;
}

void DrawRectangle(vec2 pos, vec2 size, vec3 color = vec3(1.0f), Sprite sprite = Sprite{.path = ""})
{
    // If the sprite has a path but no ID yet, resolve it once
    if (sprite.id == 0 && !sprite.path.empty())
    {
        sprite.id = GetTexture(sprite.path);
    }

    Batch *targetBatch = nullptr;
    for (auto &b : gl.world_batch)
    {
        // FAST: Integer comparison and color check
        if (b.sprite.id == sprite.id && b.color == color)
        {
            targetBatch = &b;
            break;
        }
    }

    if (targetBatch == nullptr)
    {
        gl.world_batch.push_back(Batch{.color = color, .shape = Shape::Rect, .sprite = sprite});
        targetBatch = &gl.world_batch.back();
    }

    auto vertices = CreateQuad(pos, size, vec2(0.0f), vec2(1.0f));
    targetBatch->vertices.insert(targetBatch->vertices.end(), vertices.begin(), vertices.end());
}

void DrawTextUI(Text text, vec2 pos)
{
    float startX = pos.x;                  // Keep track of the starting X for new lines
    float lineHeight = 48.0f * text.scale; // Matches the pixel size in LoadFont

    for (int i = 0; i < text.content.length(); i++)
    {
        char c = text.content[i];

        // 1. Handle Special Characters
        if (c == '\n')
        {
            pos.y -= lineHeight; // Move down
            pos.x = startX;      // Reset to start of line
            continue;
        }
        if (c == '\r')
        {
            pos.x = startX; // Carriage return
            continue;
        }
        if (c == '\t')
        {
            // Tab: Advance by 4 spaces (or any fixed amount)
            pos.x += (Characters[' '].Advance >> 6) * text.scale * 4;
            continue;
        }
        if (c == '\b')
        {
            // Backspace: Very rare in static rendering, but move cursor back
            pos.x -= (Characters[' '].Advance >> 6) * text.scale;
            continue;
        }

        // 2. Normal Character Rendering
        if (Characters.find(c) == Characters.end())
            continue;

        Character ch = Characters[c];

        float xpos = pos.x + ch.Bearing.x * text.scale;
        float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * text.scale;
        float w = ch.Size.x * text.scale;
        float h = ch.Size.y * text.scale;

        Sprite charSprite;
        charSprite.path = std::to_string(ch.texID);

        Batch *targetBatch = nullptr;
        for (auto &b : gl.ui_batch)
        {
            if (b.sprite.path == charSprite.path && b.color == text.color)
            {
                targetBatch = &b;
                break;
            }
        }

        if (!targetBatch)
        {
            gl.ui_batch.push_back(Batch{
                .color = text.color,
                .shape = Shape::Rect,
                .sprite = charSprite});
            targetBatch = &gl.ui_batch.back();
        }

        auto verts = CreateQuad({xpos, ypos}, {w, h}, {0.0f, 1.0f}, {1.0f, -1.0f});
        targetBatch->vertices.insert(targetBatch->vertices.end(), verts.begin(), verts.end());

        pos.x += (ch.Advance >> 6) * text.scale;
    }
}

// this use for load resources
void Start()
{
}

void Update(float deltaTime)
{
    DrawRectangle(vec2(50, 50), vec2(200, 200), vec3(1.0f), Sprite{.path = "assets/textures/profile.jpg"});

    // Draw text on top
    DrawTextUI(
        {.content = "Hello, World!\n@Hi",
         .scale = 1.0f,
         .color = vec3(1.0f)},
        vec2(60.0f, 90.0f));
}

void Render()
{
    gl.shader->Use();
    glBindVertexArray(gl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);

    // 1. Render World
    gl.shader->SetUniform("isText", false);
    for (const auto &batch : gl.world_batch)
    {
        gl.shader->SetUniform("baseColor", batch.color);
        gl.shader->SetUniform("useTexture", batch.sprite.id > 0);

        // No map lookup! Just use the ID directly.
        glBindTexture(GL_TEXTURE_2D, batch.sprite.id);

        glBufferData(GL_ARRAY_BUFFER, batch.vertices.size() * sizeof(Vertex), batch.vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch.vertices.size());
    }

    // 2. Render UI (Text)
    gl.shader->SetUniform("isText", true);
    gl.shader->SetUniform("useTexture", true);
    for (const auto &batch : gl.ui_batch)
    {
        gl.shader->SetUniform("baseColor", batch.color);
        u32 texID = std::stoul(batch.sprite.path);
        glBindTexture(GL_TEXTURE_2D, texID);

        glBufferData(GL_ARRAY_BUFFER, batch.vertices.size() * sizeof(Vertex), batch.vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch.vertices.size());
    }
}