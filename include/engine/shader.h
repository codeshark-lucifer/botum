#pragma once
#include <engine/utils.h>

u32 CompileShader(const char* source, i32 type);
u32 CreateShaderProgram(str vertPath, str fragPath);