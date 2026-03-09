#include <engine/gl_renderer.h>
#include <glad/glad.h>
#include <platform/win32.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stb/stb_image.h>

#include <sstream>
#include <hb.h>
#include <hb-ft.h>

GLContext gl;
std::map<u32, Character> GlyphCache;

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

bool LoadFont(const std::string &filepath)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) return false;

    FT_Face face;
    if (FT_New_Face(ft, filepath.c_str(), 0, &face)) return false;

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load every glyph present in the font file
    for (u32 glyphIndex = 0; glyphIndex < face->num_glyphs; glyphIndex++)
    {
        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER))
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
            (u32)face->glyph->advance.x
        };
        
        // Store by Glyph Index, not Character Code
        GlyphCache[glyphIndex] = character;
    }

    // Keep the face pointer if you want to use it for HarfBuzz shaping later
    // For this example, we'll assume you store it in a global or the GLContext
    gl.fontFace = face; 
    gl.hbFont = hb_ft_font_create(face, NULL);

    return true;
}

bool InitGLRender()
{
    gl.fontFace = nullptr;
    gl.hbFont = nullptr;

    if (!LoadFont("assets/fonts/arial.ttf"))
    {
        printf("Warning: Failed to load font\n");
    }

    gl.shader = new Shader("assets/shaders/scene.vert", "assets/shaders/scene.frag");
    gl.shader->Use();
    gl.shader->SetUniform("spriteTexture", 0);

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

    return true;
}

void glRender()
{
    if (!gl.shader || gl.shader->GetID() == 0)
        return;

    gl.shader->Use();
    mat4 worldProj = mat4::Ortho(0, (float)input.screen.x, 0, (float)input.screen.y, -1.0f, 1.0f);
    gl.shader->SetUniform("projection", worldProj);

    glViewport(0, 0, input.screen.x, input.screen.y);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(gl.vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);

    // 1. Render World
    gl.shader->SetUniform("isText", false);
    for (const auto &batch : gl.world_batch)
    {
        gl.shader->SetUniform("baseColor", batch.color);
        gl.shader->SetUniform("useTexture", batch.sprite.id > 0);

        glActiveTexture(GL_TEXTURE0);
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
        if (batch.sprite.path.empty())
            continue;

        u32 texID = std::stoul(batch.sprite.path);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);

        glBufferData(GL_ARRAY_BUFFER, batch.vertices.size() * sizeof(Vertex), batch.vertices.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch.vertices.size());
    }

    gl.ui_batch.clear();
    gl.world_batch.clear();
}

void DestroyGLContext()
{
    if (gl.shader)
    {
        delete gl.shader;
        gl.shader = nullptr;
    }
    if (gl.hbFont)
    {
        hb_font_destroy(gl.hbFont);
        gl.hbFont = nullptr;
    }
    glDeleteBuffers(1, &gl.vbo);
    glDeleteVertexArrays(1, &gl.vao);
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

void DrawRectangle(vec2 pos, vec2 size, vec3 color, Texture sprite)
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

void DrawTextUI(TextData text, vec2 pos, VerticalAlignment vAlign, TextAlignment hAlign)
{
    if (!gl.fontFace || !gl.hbFont) return;

    // 1. Create HarfBuzz Buffer
    hb_buffer_t *hb_buf = hb_buffer_create();
    
    // Set text and guess language/script
    hb_buffer_add_utf8(hb_buf, text.content.c_str(), -1, 0, -1);
    
    // Explicitly set script for Khmer if needed, otherwise guess
    bool hasKhmer = false;
    for (unsigned char c : text.content) {
        if (c >= 0xe0 && c <= 0xef) { // Rough check for UTF-8 Khmer range start
            hasKhmer = true;
            break;
        }
    }

    if (hasKhmer) {
        hb_buffer_set_script(hb_buf, HB_SCRIPT_KHMER);
        hb_buffer_set_language(hb_buf, hb_language_from_string("km", -1));
    }
    
    hb_buffer_guess_segment_properties(hb_buf);

    // 2. Shape the text using cached hbFont
    hb_shape(gl.hbFont, hb_buf, NULL, 0);

    // 3. Get glyph information
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(hb_buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(hb_buf, &glyph_count);

    // 4. Calculate total width for horizontal alignment
    float totalWidth = 0;
    for (unsigned int i = 0; i < glyph_count; i++) {
        totalWidth += (glyph_pos[i].x_advance >> 6) * text.scale;
    }

    // Adjust start X based on alignment
    float startX = pos.x;
    if (hAlign == TextAlignment::Center) startX -= totalWidth * 0.5f;
    else if (hAlign == TextAlignment::Right) startX -= totalWidth;

    // Adjust start Y for vertical alignment (Middle)
    float lineHeight = 48.0f * text.scale;
    if (vAlign == VerticalAlignment::Middle) pos.y -= (lineHeight * 0.25f);

    // 5. Render Pass
    float cursorX = startX;
    for (unsigned int i = 0; i < glyph_count; i++)
    {
        u32 gid = glyph_info[i].codepoint; // HarfBuzz returns Glyph ID here
        if (GlyphCache.find(gid) == GlyphCache.end()) continue;

        Character& ch = GlyphCache[gid];

        // HarfBuzz provides offsets (crucial for Khmer vowels/subscripts)
        float x_offset = (glyph_pos[i].x_offset >> 6) * text.scale;
        float y_offset = (glyph_pos[i].y_offset >> 6) * text.scale;

        float xpos = cursorX + (ch.Bearing.x * text.scale) + x_offset;
        float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * text.scale + y_offset;
        float w = ch.Size.x * text.scale;
        float h = ch.Size.y * text.scale;

        // Batching
        Texture charSprite;
        charSprite.path = std::to_string(ch.texID);

        Batch* targetBatch = nullptr;
        for (auto& b : gl.ui_batch) {
            if (b.sprite.path == charSprite.path && b.color == text.color) {
                targetBatch = &b;
                break;
            }
        }

        if (!targetBatch) {
            gl.ui_batch.push_back(Batch{.color = text.color, .sprite = charSprite});
            targetBatch = &gl.ui_batch.back();
        }

        auto verts = CreateQuad({xpos, ypos}, {w, h}, {0.0f, 1.0f}, {1.0f, -1.0f});
        targetBatch->vertices.insert(targetBatch->vertices.end(), verts.begin(), verts.end());

        // Advance cursor
        cursorX += (glyph_pos[i].x_advance >> 6) * text.scale;
        pos.y += (glyph_pos[i].y_advance >> 6) * text.scale;
    }

    // Clean up
    hb_buffer_destroy(hb_buf);
}
