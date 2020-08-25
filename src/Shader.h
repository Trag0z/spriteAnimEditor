#pragma once
#include "pch.h"

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
    GLuint sprite_dimensions_loc, set_sprite_position_on_sheet_loc;

  public:
    SheetShader() {}
    SheetShader(const char* vert_path, const char* frag_path);

    void set_sprite_dimensions(glm::vec2 dimensions) const;

    // Returns the index of the currently active sprite along the X- and Y-Axis.
    // These coordinates are used by the shader because the operations necessary
    // to calculate them from the normal sprite index (namely modulo) are only
    // supported in the more recent versions of GLSL.
    void set_sprite_position_on_sheet(glm::vec2 position) const;
};

class LineShader : public Shader {
    GLuint sprite_dimensions_loc, color_loc;

  public:
    LineShader() {}
    LineShader(const char* vert_path, const char* frag_path);

    void set_sprite_dimensions(glm::vec2 dimensions) const;
    void set_color(glm::vec4 color) const;
};