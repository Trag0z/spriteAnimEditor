#pragma once
#include "pch.h"
#include "Animation.h"

/*
    AnimationSheet text file format:
    - comments start with #
    - one variable per line, multiple values separated by commas:
        > sprite sheet path
        > sprite dimensions
        > number of animations
        > animations[]
            - name
            - number of steps
            - animation steps[]
                > sprite index
                > duration
*/

void AnimationSheet::save_to_text_file(const char* path) const {
    SDL_RWops* file_ptr = SDL_RWFromFile(path, "w");
    SDL_assert_always(file_ptr);

    size_t LINE_BUF_SIZE = MAX_SPRITE_PATH_LENGTH;
    char* line_buf = new char[LINE_BUF_SIZE];

    auto write_line = [&file_ptr, LINE_BUF_SIZE](
                          const char* data, size_t max_length = LINE_BUF_SIZE) {
        size_t length = strnlen_s(data, max_length);
        SDL_RWwrite(file_ptr, data, sizeof(char), length);
        SDL_RWwrite(file_ptr, "\n", sizeof(char), 1);
    };

    write(sprite_path);

    sprintf_s(line_buf, LINE_BUF_SIZE, "%d,%d", sprite_dimensions.x,
              sprite_dimensions.y);
    write(line_buf);

    _itoa_s(animations.size(), line_buf, LINE_BUF_SIZE, 10);
    write(line_buf);

    for (const auto& anim : animations) {
        write(anim.name, Animation::MAX_NAME_LENGTH);

        _itoa_s(anim.steps.size(), line_buf, LINE_BUF_SIZE, 10);
        write(line_buf);

        for (const auto& step : anim.steps) {
            _itoa_s(step.sprite_index, line_buf, LINE_BUF_SIZE, 10);
            write(line_buf);

            sprintf_s(line_buf, LINE_BUF_SIZE, "%f", step.duration);
            write(line_buf);
        }
    }

    // @CLEANUP: Is this necessary?
    SDL_RWwite(file_ptr, '\0', sizeof(char), 1);
    SDL_RWclose(file_ptr);

    delete[] word_buf;
}

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
    auto read_word = [&file_buf, word_buf,
                      WORD_BUF_SIZE](char delim = '\n ') -> size_t {
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
            // SDL_assert_always(*file_buf != '\0');
        }
        // Add deliminating null character and advance file_buf pointer
        SDL_assert_always(num_chars_written < WORD_BUF_SIZE);
        *word_buf = '\0';
        ++file_buf;
        return num_chars_written;
    };

    // Read sprite sheet path
    size_t sprite_path_length = read_word();
    if (sprite_path) {
        delete[] sprite_path;
    }
    sprite_path = new char[sprite_path_length];
    strcpy_s(sprite_path, sprite_path_length, word_buf);

    sprite_sheet.load_from_file(sprite_path);

    // Read sprite dimensions
    read_word(',');
    sprite_dimensions.x = atoi(word_buf);
    read_word();
    sprite_dimensions.y = atoi(word_buf);

    num_sprites = (sprite_sheet.dimensions.x / sprite_dimensions.x) *
                  (sprite_sheet.dimensions.y / sprite_dimensions.y);

    // Read animations
    read_word();
    size_t num_animations = atof(word_buf);

    animations.clear();
    animations.reserve(num_animations);

    for (size_t n_animation = 0; n_animation < num_animations; ++n_animation) {
        Animation anim;
        read_word();
        strncpy_s(anim.name, word_buf, Animation::MAX_NAME_LENGTH);

        read_word();
        size_t num_steps = atof(word_buf);
        anim.steps.reserve(num_steps);

        for (size_t n_step = 0; n_step < num_steps; ++n_step) {
            Animation::AnimationStepData step;

            read_word();
            step.sprite_index = atoi(word_buf);
            read_word();
            step.duration = atof(word_buf);
            anim.steps.push_back(step);
        }
        animations.push_back(anim);
    }

    delete[] word_buf;

    SDL_RWclose(file_ptr);
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
