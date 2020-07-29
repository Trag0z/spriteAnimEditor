#pragma once
#include "pch.h"

struct Texture {
    GLuint id;
    glm::ivec2 dimensions;

    void load_from_file(const char* path);
};