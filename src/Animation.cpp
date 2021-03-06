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

    const size_t TEXT_BUF_SIZE = 1024;
    char text_buf[TEXT_BUF_SIZE];

    int length =
        sprintf_s(text_buf, TEXT_BUF_SIZE,
                  "# Sprite sheet path\n%s\n# Dimensions\n%d,%d\n# Number of "
                  "animations\n%zu\n",
                  png_file_name, sprite_dimensions.x, sprite_dimensions.y,
                  animations.size());
    SDL_assert(length != -1);
    SDL_RWwrite(file_ptr, text_buf, sizeof(char), length);

    for (const auto& anim : animations) {
        length = sprintf_s(text_buf, "# Animation\n%s\n%zu\n# Steps\n",
                           anim.name, anim.steps.size());
        SDL_assert(length != -1);
        SDL_RWwrite(file_ptr, text_buf, sizeof(char), length);

        for (const auto& step : anim.steps) {
            length = sprintf_s(text_buf, "%d\n%f\n", step.sprite_index,
                               step.duration);
            SDL_assert(length != -1);
            SDL_RWwrite(file_ptr, text_buf, sizeof(char), length);
        }
    }

    SDL_RWclose(file_ptr);
}

void AnimationSheet::load_from_text_file(const char* path) {
    SDL_RWops* file_ptr = SDL_RWFromFile(path, "r");
    SDL_assert_always(file_ptr);

    glm::i64 file_size = SDL_RWsize(file_ptr);
    // Keep this pointer to the start of the buffer so it can be deleted later
    char* file_buf = new char[file_size];
    char* next_char = file_buf;

    SDL_RWread(file_ptr, next_char, sizeof(char), file_size);

    const size_t WORD_BUF_SIZE = MAX_SPRITE_PATH_LENGTH;
    char word_buf[WORD_BUF_SIZE];

    // Reads characters from next_char into word_buf (while moving next_char)
    // until delim is found or end of file is reached. Returns number of
    // characters written (including deliminating \0 character).
    auto read_word = [&next_char, WORD_BUF_SIZE](char* dst_buf,
                                                 char delim = '\n') -> size_t {
        while (*next_char == '#') {
            // Comment, skip line
            while (*next_char != '\n') {
                if (*next_char == '\0') {
                    // This should never happen if we read files that were
                    // written by this application
                    SDL_TriggerBreakpoint();
                    return 0;
                }
                ++next_char;
            }
        }
        if (*next_char == '\r' || *next_char == '\n') {
            ++next_char;
        }

        size_t num_chars_written = 0;
        while (*next_char != delim && num_chars_written != WORD_BUF_SIZE - 1) {
            if (*next_char == '\r') {
                // To deal with different end of line sequences
                ++next_char;
                continue;
            }
            *dst_buf++ = *next_char++;
            ++num_chars_written;
        }
        // Add deliminating null character and advance next_char pointer
        SDL_assert_always(num_chars_written < WORD_BUF_SIZE);
        *dst_buf = '\0';
        ++next_char;
        return ++num_chars_written;
    };

    // Read png file name
    size_t png_name_length = read_word(word_buf);
    if (png_file_name) {
        delete[] png_file_name;
    }
    png_file_name = new char[png_name_length];
    strcpy_s(png_file_name, png_name_length, word_buf);

    // Create full path to png from path and png_file_name
    std::string png_path(path);
    size_t last_slash = png_path.find_last_of('\\');
    png_path.erase(last_slash);
    png_path.append(png_file_name);

    sprite_sheet.load_from_file(png_path.c_str());

    // Read sprite dimensions
    read_word(word_buf, ',');
    sprite_dimensions.x = atoi(word_buf);
    read_word(word_buf);
    sprite_dimensions.y = atoi(word_buf);

    num_sprites = (sprite_sheet.dimensions.x / sprite_dimensions.x) *
                  (sprite_sheet.dimensions.y / sprite_dimensions.y);

    // Read animations
    read_word(word_buf);
    size_t num_animations = atoi(word_buf);

    animations.clear();
    animations.reserve(num_animations);

    for (size_t n_animation = 0; n_animation < num_animations; ++n_animation) {
        Animation anim;
        read_word(word_buf);
        strncpy_s(anim.name, word_buf, Animation::MAX_NAME_LENGTH);

        read_word(word_buf);
        size_t num_steps = atoi(word_buf);
        anim.steps.reserve(num_steps);

        for (size_t n_step = 0; n_step < num_steps; ++n_step) {
            Animation::AnimationStepData step;

            read_word(word_buf);
            step.sprite_index = atoi(word_buf);
            read_word(word_buf);
            step.duration = static_cast<float>(atof(word_buf));
            anim.steps.push_back(step);
        }
        animations.push_back(anim);
    }

    delete[] file_buf;

    SDL_RWclose(file_ptr);
}

template <typename T> static T greatest_common_divisor(T a, T b) {
    if (b == 0)
        return a;
    return greatest_common_divisor(b, a % b);
}

void AnimationSheet::create_new_from_png(const char* path) {
    const char* sprite_name = strrchr(path, '\\');

    if (png_file_name) {
        delete[] png_file_name;
    }
    size_t length = strnlen_s(sprite_name, MAX_SPRITE_PATH_LENGTH) + 1;
    png_file_name = new char[length];
    strcpy_s(png_file_name, length, sprite_name);

    sprite_sheet.load_from_file(path);

    // Make a reasonable guess at the new sprite sheets sprite dimensions
    sprite_dimensions = glm::ivec2(greatest_common_divisor(
        sprite_sheet.dimensions.x, sprite_sheet.dimensions.y));

    num_sprites = (sprite_sheet.dimensions.x / sprite_dimensions.x) *
                  (sprite_sheet.dimensions.y / sprite_dimensions.y);

    animations.clear();
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

int AnimationPreview::get_sprite_index() {
    SDL_assert(animation);
    if (animation == nullptr || animation->steps.size() == 0) {
        return -1;
    }
    return animation->steps[current_step].sprite_index;
}
