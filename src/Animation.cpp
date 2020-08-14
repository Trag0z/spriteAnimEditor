#pragma once
#include "pch.h"
#include "Animation.h"

/*
    AnimationSheet text file format:
    - comments start with #
    - one variable per line, multiple values separated by commas:
        > sprite sheet path
        > sprite dimensions
        > animations[]
            - name
            - animation steps[]
                > sprite index
                > duration
*/

void AnimationSheet::load_from_text_file(const char* path) {
    SDL_RWops* file_ptr = SDL_RWFromFile(path, "r");
    SDL_assert_always(file_ptr);

    s64 file_size = SDL_RWsize(file_ptr);
    char* file_buf = new char[file_size];

    SDL_RWread(file_ptr, file_buf, sizeof(char), file_size);

    const size_t WORD_BUF_SIZE = MAX_SPRITE_PATH_LENGTH;
    char word_buf[WORD_BUF_SIZE];

    // Reads characters from file_buf into word_buf (while moving file_buf)
    // until delim is found or end of file is reached. Returns number of
    // characters written (including deliminating \0 character).
    auto read_to_delim = [&file_buf, WORD_BUF_SIZE](char* word_buf,
                                                    char delim) -> size_t {
        while (*file_buf == '#') {
            // Comment, skip line
            while (*file_buf != '\n') {
                if (*file_buf == '\0') {
                    return 0;
                }
                ++file_buf;
            }
        }

        size_t num_chars_written = 0;
        while (*file_buf != delim && num_chars_written != WORD_BUF_SIZE - 1) {
            *word_buf++ = *file_buf++;
            ++num_chars_written;
        }
        // Add deliminating null character and advance file_buf pointer
        SDL_assert_always(num_chars_written < WORD_BUF_SIZE);
        *word_buf = '\0';
        ++file_buf;
        return num_chars_written;
    };

    // Read sprite sheet path
    size_t sprite_path_length = read_to_delim(word_buf, '\n');
    if (sprite_path) {
        delete[] sprite_path;
    }
    sprite_path = new char[sprite_path_length];
    strcpy_s(sprite_path, sprite_path_length, word_buf);

    sprite_sheet.load_from_file(sprite_path);

    // Read sprite dimensions
    read_to_delim(word_buf, ',');
    // NEXT: atof() or something?
}

void AnimationPreview::set_animation(const Animation* anim) {
    animation = anim;
    current_step = 0;
    current_duration = 0.0f;
}

void AnimationPreview::update(float delta_time) {
    if (animation == nullptr || animation->steps.size() == 0) {
        return;
    }

    if (current_step >= animation->steps.size()) {
        current_step = 0;
        current_duration = 0.0f;
    }

    current_duration += delta_time;
    float sprite_duration = animation->steps[current_step].duration;

    if (current_duration > sprite_duration) {
        current_duration -= sprite_duration;
        ++current_step;

        if (current_step == animation->steps.size()) {
            current_step = 0;
        }
    }
}

s64 AnimationPreview::get_sprite_index() {
    if (animation == nullptr || animation->steps.size() == 0) {
        return -1;
    }
    return animation->steps[current_step].sprite_index;
}
