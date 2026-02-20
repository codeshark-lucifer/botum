#version 430 core

layout (location = 0) in vec2 textureCoordsIn;
layout (location = 1) in vec4 baseColor;

layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D textureAtlas;

uniform int isFont;

void main()
{
    if (isFont == 2) 
    {
        fragColor = baseColor;
    }
    else if(isFont == 1)
    {
        float alpha = texture(textureAtlas, textureCoordsIn).r;
        if(alpha < 0.01) discard;
        fragColor = vec4(baseColor.rgb, alpha); 
    }
    else
    {
        vec4 texColor = texture(textureAtlas, textureCoordsIn);
        if(texColor.a < 0.01) discard;
        fragColor = texColor * baseColor;
    }
}