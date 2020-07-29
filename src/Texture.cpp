#pragma once
#include "pch.h"
#include "Texture.h"

void Texture::load_from_file(const char* path) {
    glDeleteTextures(1, &id);

    SDL_Surface* img = IMG_Load(path);
    SDL_assert(img);

    dimensions.x = img->w;
    dimensions.y = img->h;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
    // NOTE: Is this actually useful?
    // glGenerateMipmap(GL_TEXTURE_2D);
    SDL_FreeSurface(img);

    // Set Texture wrap and filter modes
    // NOTE: Is this specific to one texture or a global setting?
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // @OPTIMIZATION: delete this
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}