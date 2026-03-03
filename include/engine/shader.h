#pragma once
#include "utils.h"

class Shader
{
public:
    Shader(const str &vertPath, const str &fragPath);
    ~Shader();

    void Use();
    
    template<typename T>
    void SetUniform(str name, T value);

private:
    u32 CompileShader(i32 type, const char *source);
    u32 CreateShaderProgram(str vertPath, str fragPath);
private:
    u32 id = 0;
};
