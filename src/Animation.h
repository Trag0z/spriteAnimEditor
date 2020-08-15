#pragma once
#include "pch.h"
#include "Texture.h"
#include "Types.h"

struct Animation {
    static const size_t MAX_NAME_LENGTH = 64;

    char name[MAX_NAME_LENGTH] = "\n";

    struct AnimationStepData {
        int sprite_index;
        float duration;
    };

    std::vector<AnimationStepData> steps;
};

struct AnimationSheet {
    char* sprite_path;
    Texture sprite_sheet;
    glm::ivec2 sprite_dimensions;
    size_t num_sprites;

    std::vector<Animation> animations;

    void save_to_text_file(const char* path) const;
    void load_from_text_file(const char* path);
    void create_new_from_png(const char* path);

    static const size_t MAX_SPRITE_PATH_LENGTH = 256;
};

class AnimationPreview {
    const Animation* animation;
    size_t current_step;
    float current_duration;

  public:
    void set_animation(const Animation* anim);
    void update(float delta_time);
    s64 get_sprite_index();
};