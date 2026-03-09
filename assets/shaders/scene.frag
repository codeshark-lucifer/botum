#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 baseColor;
uniform bool isText;
uniform bool useTexture;
uniform sampler2D spriteTexture;

void main()
{    
    if(isText)
    {
        // FreeType renders glyphs to a GL_RED texture. 
        // We use that red value as our alpha mask.
        float alpha = texture(spriteTexture, TexCoords).r;
        FragColor = vec4(baseColor, alpha);
    }
    else if(useTexture)
    {
        FragColor = texture(spriteTexture, TexCoords) * vec4(baseColor, 1.0);
    }
    else
    {
        // Solid colored shapes
        FragColor = vec4(baseColor, 1.0);
    }
}