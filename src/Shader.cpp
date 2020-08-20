#pragma once
#include "pch.h"
#include "Shader.h"

static bool check_compile_errors(GLuint object, bool program) {
    GLint success;
    GLchar infoLog[1024];

    if (program) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            printf("ERROR: Program link-time error:\n%s", infoLog);
            return false;
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            printf("ERROR: Shader compile-time error:\n%s", infoLog);
            return false;
        }
    }
    return true;
}

// Returns -1 on error
GLuint load_and_compile_shader_from_file(const char* vert_shader_path,
                                         const char* frag_shader_path) {
    std::string vert_shader_string;
    std::string frag_shader_string;
    try {
        std::ifstream vert_shader_file(vert_shader_path);
        std::ifstream frag_shader_file(frag_shader_path);

        std::stringstream vert_shader_stream;
        std::stringstream frag_shader_stream;

        vert_shader_stream << vert_shader_file.rdbuf();
        frag_shader_stream << frag_shader_file.rdbuf();

        vert_shader_file.close();
        frag_shader_file.close();

        vert_shader_string = vert_shader_stream.str();
        frag_shader_string = frag_shader_stream.str();
    } catch (std::exception e) {
        printf("ERROR: Failed to read shader file/n");
        SDL_assert_always(false);
    }

    const GLchar* vert_shader_c_str = vert_shader_string.c_str();
    const GLchar* frag_shader_c_str = frag_shader_string.c_str();

    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_shader_c_str, NULL);
    glCompileShader(vert_shader);
    if (!check_compile_errors(vert_shader, false))
        SDL_assert_always(false);

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_shader_c_str, NULL);
    glCompileShader(frag_shader);
    if (!check_compile_errors(frag_shader, false))
        SDL_assert_always(false);

    GLuint program_id = glCreateProgram();

    glAttachShader(program_id, vert_shader);
    glAttachShader(program_id, frag_shader);

    glLinkProgram(program_id);
    if (!check_compile_errors(program_id, true))
        SDL_assert_always(false);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program_id;
}

Shader::Shader(const char* vert_path, const char* frag_path) {
    id = load_and_compile_shader_from_file(vert_path, frag_path);
    projection_loc = glGetUniformLocation(id, "projection");
    render_position_loc = glGetUniformLocation(id, "render_position");
}

void Shader::use() const { glUseProgram(id); }

void Shader::set_projection(glm::mat4 projection) const {
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, value_ptr(projection));
}

void Shader::set_render_position(glm::vec2 position) const {
    glUniform2fv(render_position_loc, 1, value_ptr(position));
}

SheetShader::SheetShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    sprite_dimensions_loc = glGetUniformLocation(id, "sprite_dimensions");
    set_sprite_position_on_sheet_loc = glGetUniformLocation(id, "sprite_position_on_sheet");
}

void SheetShader::set_sprite_dimensions(glm::vec2 dimensions) const {
    glUniform2fv(sprite_dimensions_loc, 1, value_ptr(dimensions));
}
void SheetShader::set_sprite_position_on_sheet(glm::vec2 position) const {
    glUniform2fv(set_sprite_position_on_sheet_loc, 1, value_ptr(position));
}

LineShader::LineShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    sprite_dimensions_loc = glGetUniformLocation(id, "sprite_dimensions");
    color_loc = glGetUniformLocation(id, "color");
}

void LineShader::set_sprite_dimensions(glm::vec2 dimensions) const {
    glUniform2fv(sprite_dimensions_loc, 1, value_ptr(dimensions));
}

void LineShader::set_color(glm::vec4 color) const {
    glUniform4fv(color_loc, 1, value_ptr(color));
}