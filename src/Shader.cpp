#pragma once
#include "pch.h"
#include "Shader.h"

static bool checkCompileErrors(GLuint object, bool program) {
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
GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
                                    const char* fShaderPath) {
    std::string vShaderString;
    std::string fShaderString;
    try {
        std::ifstream vShaderFile(vShaderPath);
        std::ifstream fShaderFile(fShaderPath);

        std::stringstream vShaderStream;
        std::stringstream fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vShaderString = vShaderStream.str();
        fShaderString = fShaderStream.str();
    } catch (std::exception e) {
        printf("ERROR: Failed to read shader file/n");
        SDL_assert_always(false);
    }

    const GLchar* vShaderSource = vShaderString.c_str();
    const GLchar* fShaderSource = fShaderString.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    if (!checkCompileErrors(vShader, false))
        SDL_assert_always(false);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSource, NULL);
    glCompileShader(fShader);
    if (!checkCompileErrors(fShader, false))
        SDL_assert_always(false);

    GLuint id = glCreateProgram();

    glAttachShader(id, vShader);
    glAttachShader(id, fShader);

    glLinkProgram(id);
    if (!checkCompileErrors(id, true))
        SDL_assert_always(false);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return id;
}

Shader::Shader(const char* vert_path, const char* frag_path) {
    id = loadAndCompileShaderFromFile(vert_path, frag_path);
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
    sprite_index_loc = glGetUniformLocation(id, "sprite_index");
}

void SheetShader::set_sprite_dimensions(glm::vec2 dimensions) const {
    glUniform2fv(sprite_dimensions_loc, 1, value_ptr(dimensions));
}
void SheetShader::set_sprite_index(GLint index) const {
    glUniform1i(sprite_index_loc, index);
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