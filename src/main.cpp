#include <platform/win32.h>
#include <glad/glad.h>
#include <engine/shader.h>
#include <mathf/matries.h>
#include <stb/stb_image.h>

struct Vertex
{
    vec2 position;
    vec2 uv;
};

void CreateQuad(vec2 pos, vec2 size, array<Vertex>* vertices, array<u32>* indices)
{
    u32 start = vertices->size();

    vertices->push_back(Vertex{vec2(pos.x, pos.y),              vec2(0.0f, 1.0f)}); // bottom-left
    vertices->push_back(Vertex{vec2(pos.x, pos.y + size.y),     vec2(0.0f, 0.0f)}); // top-left
    vertices->push_back(Vertex{vec2(pos.x + size.x, pos.y + size.y), vec2(1.0f, 0.0f)}); // top-right
    vertices->push_back(Vertex{vec2(pos.x + size.x, pos.y),     vec2(1.0f, 1.0f)}); // bottom-right

    indices->push_back(start + 0);
    indices->push_back(start + 1);
    indices->push_back(start + 2);

    indices->push_back(start + 2);
    indices->push_back(start + 3);
    indices->push_back(start + 0);
}

int main()
{
    InitPlatform();
    CreateWindowPlatform("botum", 956, 540);

    Shader shader("assets/shaders/scene.vert", "assets/shaders/scene.frag");

    array<Vertex> vertices = {};
    array<u32> indices = {};

    CreateQuad(vec2(0.0f), vec2(1.0f), &vertices, &indices);

    u32 vao = 0, vbo = 0, ebo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Vertex) * vertices.size(),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(u32) * indices.size(),
                 indices.data(),
                 GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );

    // UV attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, uv)
    );

    // Texture
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(
        "assets/textures/sprite.png",
        &width,
        &height,
        &nrChannels,
        0
    );

    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        );

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else
    {
        unsigned char fallback_data[] =
        {
            255,0,255,255, 0,0,0,255,
            0,0,0,255, 255,0,255,255
        };

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            2,
            2,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            fallback_data
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!ShouldClose())
    {
        Event e;
        PollEvent(&e);

        glViewport(0, 0, input.screen.x, input.screen.y);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (float)input.screen.x / (float)input.screen.y;

        mat4 projection = mat4::Ortho(
            -aspect,
            aspect,
            -1.0f,
            1.0f,
            -1.0f,
            1.0f
        );

        shader.Use();
        shader.SetUniform("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        shader.SetUniform("texture1", 0);

        glBindVertexArray(vao);

        glDrawElements(
            GL_TRIANGLES,
            indices.size(),
            GL_UNSIGNED_INT,
            0
        );

        SwapBuffersWindow();
    }

    DestroyPlatform();
    return 0;
}