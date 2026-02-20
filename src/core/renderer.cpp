#include <engine/gl_renderer.hpp>
#include <glad/glad.h>

#include <freetype/freetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

GLContext gl;

bool InitGLRenderer(){
    renderData = BumpAlloc<RenderData>(&persistentStorage);
    return true;
}

void glRender(){
    
}

void DestroyGLContext(){
    
}

