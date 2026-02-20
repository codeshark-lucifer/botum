#include <engine/gl_renderer.hpp>
#include <glad/glad.h>

#include <freetype/freetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

GLContext gl;
Font font;

bool LoadFont(const char *path, int pixelSize)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        return false;

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face))
        return false;

    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    // store ascender (in pixels) for baseline calculations
    font.pixelSize = pixelSize;
    if (face->size)
        font.ascender = (face->size->metrics.ascender >> 6);
    else
        font.ascender = pixelSize; // fallback

    const int firstChar = 32;
    const int lastChar = 126;

    int atlasWidth = 0;
    int atlasHeight = 0;

    const int padding = 2;
    for (int c = firstChar; c <= lastChar; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        atlasWidth += face->glyph->bitmap.width + padding;
        atlasHeight = std::max(atlasHeight,
                               (int)face->glyph->bitmap.rows);
    }

    font.atlasWidth = atlasWidth;
    font.atlasHeight = atlasHeight;
    font.pixelSize = pixelSize;

    std::vector<unsigned char> atlas(atlasWidth * atlasHeight);

    int x = 0;

    for (int c = firstChar; c <= lastChar; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        FT_Bitmap &bmp = face->glyph->bitmap;

        for (int row = 0; row < bmp.rows; row++)
            for (int col = 0; col < bmp.width; col++)
            {
                int index = row * atlasWidth + (x + col);
                atlas[index] = bmp.buffer[row * bmp.pitch + col];
            }

        Glyph glyph;
        glyph.size = ivec2(bmp.width, bmp.rows);
        glyph.bearing = ivec2(face->glyph->bitmap_left,
                              face->glyph->bitmap_top);
        glyph.advance = face->glyph->advance.x;
        glyph.offset = ivec2(x, 0);

        font.glyphs.insert({(char)c, glyph});

        x += bmp.width + padding;
    }

    glGenTextures(1, &font.texture);
    glBindTexture(GL_TEXTURE_2D, font.texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                 atlasWidth, atlasHeight,
                 0, GL_RED, GL_UNSIGNED_BYTE,
                 atlas.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return true;
}

void PushSprite(ivec2 offset, ivec2 size, vec2 pos, vec2 renderSize, vec3 color, int renderOptions, float layer)
{
    renderData->transforms.push_back(Transform{
        .ioffset = offset,
        .isize = size,
        .pos = pos,
        .size = renderSize,
        .color = vec4(color, 1.0f),
        .renderOptions = renderOptions,
        .layer = layer,
        ._padding = {0, 0}});
}

bool InitGLRenderer()
{
    renderData = BumpAlloc<RenderData>(&persistentStorage);
    if (!LoadFont(FONT_FILE_PATH, 225))
    {
        print("Failed to load font!\n");
        return false;
    }
    gl.shader = CreateShaderProgram("assets/shaders/scene.vert",
                                    "assets/shaders/scene.frag");
    glUseProgram(gl.shader);

    // --- Camera ---
    Camera2D camera;
    camera.dimensions = vec2(input->screen.x, input->screen.y);
    glUniformMatrix4fv(glGetUniformLocation(gl.shader, "projection"), 1, GL_FALSE, camera.matrix().m);

    // --- Load texture atlas ---
    i32 texChannels;
    u8 *imageData = stbi_load("assets/sprites/sample.png", &gl.texSize.x, &gl.texSize.y, &texChannels, 4);
    Assert(imageData, "Failed to load texture");

    glGenTextures(1, &gl.atlasTex);
    glBindTexture(GL_TEXTURE_2D, gl.atlasTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl.texSize.x, gl.texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(imageData);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(gl.shader, "isFont"), 0);
    glUniform1i(glGetUniformLocation(gl.shader, "textureAtlas"), 0);
    glUniform2f(glGetUniformLocation(gl.shader, "atlasSize"), (float)gl.texSize.x, (float)gl.texSize.y);
    glUniform1i(glGetUniformLocation(gl.shader, "isFont"), 0);

    // --- VAO (unit quad handled via gl_VertexID in shader) ---
    glGenVertexArrays(1, &gl.vao);
    glBindVertexArray(gl.vao);
    glBindVertexArray(0);

    // --- SSBO ---
    glGenBuffers(1, &gl.transSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl.transSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl.transSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 sizeof(Transform) * renderData->transforms.size(),
                 renderData->transforms.data(),
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // --- GL states ---
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    return true;
}

void glRender()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(gl.vao);

    glUseProgram(gl.shader);
    glDisable(GL_DEPTH_TEST);
    
    // Render Text
    if (!renderData->uiTransforms.empty())
    {
        glUniform2f(glGetUniformLocation(gl.shader, "atlasSize"), font.atlasWidth, font.atlasHeight);
        glUniform1i(glGetUniformLocation(gl.shader, "isFont"), 1);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl.transSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * renderData->uiTransforms.size(), renderData->uiTransforms.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl.transSSBO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font.texture);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)renderData->uiTransforms.size());
    }

    // --- Clear transforms for next frame ---
    renderData->transforms.clear();
    renderData->uiTransforms.clear();
    glBindVertexArray(0);
}

void DestroyGLContext()
{
    glDeleteBuffers(1, &gl.transSSBO);
    glDeleteTextures(1, &gl.atlasTex);
    glDeleteProgram(gl.shader);
}