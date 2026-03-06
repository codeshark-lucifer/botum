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

int main()
{
    InitPlatform();
    CreateWindowPlatform("botum", 956, 540);

    Shader shader("assets/shaders/scene.vert", "assets/shaders/scene.frag");

    // Sprite vertices (Two Quads: 12 vertices)
    // Position (x, y), UV (u, v)
    array<Vertex> vertices = {
        // First Quad (Left)
        {vec2(-0.8f,  0.3f), vec2(0.0f, 1.0f)}, 
        {vec2(-0.8f, -0.3f), vec2(0.0f, 0.0f)}, 
        {vec2(-0.2f, -0.3f), vec2(1.0f, 0.0f)}, 

        {vec2(-0.8f,  0.3f), vec2(0.0f, 1.0f)}, 
        {vec2(-0.2f, -0.3f), vec2(1.0f, 0.0f)}, 
        {vec2(-0.2f,  0.3f), vec2(1.0f, 1.0f)},

        // Second Quad (Right)
        {vec2( 0.2f,  0.3f), vec2(0.0f, 1.0f)}, 
        {vec2( 0.2f, -0.3f), vec2(0.0f, 0.0f)}, 
        {vec2( 0.8f, -0.3f), vec2(1.0f, 0.0f)}, 

        {vec2( 0.2f,  0.3f), vec2(0.0f, 1.0f)}, 
        {vec2( 0.8f, -0.3f), vec2(1.0f, 0.0f)}, 
        {vec2( 0.8f,  0.3f), vec2(1.0f, 1.0f)}
    };

    u32 vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));

    // UV attribute (location 2 in shader)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, uv)));

    // Load Texture
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/textures/sprite.png", &width, &height, &nrChannels, 0);
    
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        // Fallback: Magenta/Black checkerboard
        unsigned char fallback_data[] = {
            255, 0, 255, 255,   0, 0, 0, 255,
            0, 0, 0, 255,       255, 0, 255, 255
        };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback_data);
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

        float aspect = static_cast<float>(input.screen.x) / static_cast<float>(input.screen.y);
        // Orthographic projection: left, right, bottom, top, near, far
        mat4 projection = mat4::Ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

        shader.Use();
        shader.SetUniform("projection", projection);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.SetUniform("texture1", 0);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        SwapBuffersWindow();
    }
    DestroyPlatform();
    return 0;
}