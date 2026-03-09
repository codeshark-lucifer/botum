#pragma once
#include "utils.h"
#include <mathf/matries.h>

class Shader
{
public:
    Shader(const str &vertPath, const str &fragPath);
    ~Shader();

    void Use();
    u32 GetID() const { return id; }
    
    template<typename T>
    void SetUniform(str name, T value);

private:
    u32 CompileShader(i32 type, const char *source);
    u32 CreateShaderProgram(str vertPath, str fragPath);
private:
    u32 id = 0;
};

template <> void Shader::SetUniform<float>(str name, float value);
template <> void Shader::SetUniform<int>(str name, int value);
template <> void Shader::SetUniform<bool>(str name, bool value);
template <> void Shader::SetUniform<mat4>(str name, mat4 value);
