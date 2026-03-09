#include <engine/shader.h>
#include <glad/glad.h>
#include <cstdio>
#include <mathf/matries.h>
#include <mathf/vectors.h>

Shader::Shader(const str &vertPath, const str &fragPath)
{
    id = CreateShaderProgram(vertPath, fragPath);
}

Shader::~Shader()
{
    glDeleteProgram(id);
}

u32 Shader::CompileShader(i32 type, const char *source)
{
    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    i32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        printf("Failed to Compile Shader(%s): %s",
               type == GL_VERTEX_SHADER ? "Vertex" : "Fragment",
               log);
    }
    return shader;
}

u32 Shader::CreateShaderProgram(str vertPath, str fragPath)
{
    char *vertSource = read_file(vertPath);
    char *fragSource = read_file(fragPath);

    if (!vertSource || !fragSource)
    {
        printf("Failed to read shader files!");
        if (vertSource)
            free(vertSource);
        if (fragSource)
            free(fragSource);
        return 0;
    }

    u32 vertexShader = CompileShader(GL_VERTEX_SHADER, vertSource);
    u32 fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragSource);

    free(vertSource);
    free(fragSource);

    u32 program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    i32 success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        printf("Failed to Link Program Shader: %s", log);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void Shader::Use()
{
    glUseProgram(id);
}

template <>
void Shader::SetUniform<float>(str name, float value)
{
    u32 loc = glGetUniformLocation(id, name.c_str());
    glUniform1f(loc, value);
}

template <>
void Shader::SetUniform<int>(str name, int value)
{
    u32 loc = glGetUniformLocation(id, name.c_str());
    glUniform1i(loc, value);
}

template <>
void Shader::SetUniform<bool>(str name, bool value)
{
    u32 loc = glGetUniformLocation(id, name.c_str());
    glUniform1i(loc, value);
}

template <>
void Shader::SetUniform<vec3>(str name, vec3 value)
{
    u32 loc = glGetUniformLocation(id, name.c_str());
    glUniform3f(loc, value.x, value.y, value.z);
}

template <>
void Shader::SetUniform<mat4>(str name, mat4 value)
{
    u32 loc = glGetUniformLocation(id, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, value.m); // Changed GL_TRUE to GL_FALSE
}