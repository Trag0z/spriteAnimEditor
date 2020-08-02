#pragma once
#include "pch.h"

static bool checkCompileErrors(GLuint object, bool program);

GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
                                    const char* fShaderPath);

class Shader {
  protected:
    GLuint id, projection_loc, render_position_loc;

  public:
    Shader() {}
    Shader(const char* vert_path, const char* frag_path);

    void use() const;

    void set_projection(glm::mat4 projection) const;
    void set_render_position(glm::vec2 position) const;
};

class SheetShader : public Shader {
    GLuint sprite_dimensions_loc, sprite_index_loc;

  public:
    SheetShader() {}
    SheetShader(const char* vert_path, const char* frag_path);

    void set_sprite_dimensions(glm::vec2 dimensions) const;
    void set_sprite_index(GLint index) const;
};

class LineShader : public Shader {
    GLuint sprite_dimensions_loc, color_loc;

  public:
    LineShader() {}
    LineShader(const char* vert_path, const char* frag_path);

    void set_sprite_dimensions(glm::vec2 dimensions) const;
    void set_color(glm::vec4 color) const;
};