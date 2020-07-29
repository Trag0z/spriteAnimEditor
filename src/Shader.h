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
    Shader(const char* vert_path, const char* frag_path);

  public:
    void use() const;
};

class SheetShader : public Shader {
    GLuint sprite_dimensions_loc, sprite_index_loc;

  public:
    SheetShader() {}
    SheetShader(const char* vert_path, const char* frag_path);

    void set_sprite_dimensions(glm::ivec2 dimensions);
    void set_sprite_index(GLint index);
};