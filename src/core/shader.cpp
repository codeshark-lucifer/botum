#include <engine/shader.h>
#include <glad/glad.h>

u32 CompileShader(const char* source, i32 type) {
    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    i32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        print("Failed to Compile Shader(%s): %s",
              type == GL_VERTEX_SHADER ? "Vertex" : "Fragment",
              log);
    }
    return shader;
}

u32 CreateShaderProgram(str vertPath, str fragPath) {
    char* vertSource = read_file(vertPath);
    char* fragSource = read_file(fragPath);

    if (!vertSource || !fragSource) {
        print("Failed to read shader files!");
        if (vertSource) free(vertSource);
        if (fragSource) free(fragSource);
        return 0;
    }

    u32 vertexShader = CompileShader(vertSource, GL_VERTEX_SHADER);
    u32 fragmentShader = CompileShader(fragSource, GL_FRAGMENT_SHADER);

    free(vertSource);
    free(fragSource);

    u32 program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    i32 success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        print("Failed to Link Program Shader: %s", log);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}