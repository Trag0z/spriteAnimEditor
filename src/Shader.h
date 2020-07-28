#pragma once
#include "pch.h"

static bool checkCompileErrors(GLuint object, bool program);

GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
                                    const char* fShaderPath);

class Shader {
  protected:
    GLuint id;

  public:
    Shader() {}
    Shader(const char* vert_path, const char* frag_path) {
        id = loadAndCompileShaderFromFile(vert_path, frag_path);
    }

  public:
    void use() const { glUseProgram(id); };
};