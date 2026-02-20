#pragma once

#include <engine/shader.h>
#include <stb/stb_image.h>
#include <engine/render_types.h>
#include <platform/win32.h>
#include <string>
#include <map>

constexpr str FONT_FILE_PATH     = "assets/fonts/arial.ttf";
constexpr str SHADER_SCENE_VERT  = "assets/shaders/scene.vert";
constexpr str SHADER_SCENE_FRAG  = "assets/shaders/scene.frag";

struct GLContext
{
    u32 shader;
    u32 vao;
    u32 transSSBO;
};

extern GLContext gl;

bool InitGLRenderer();
void glRender();
void DestroyGLContext();