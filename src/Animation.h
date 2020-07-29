#pragma once
#include "pch.h"

struct Animation {
    static const size_t MAX_NAME_LENGTH = 64;

    char name[MAX_NAME_LENGTH] = "\n";

    struct AnimationStepData {
        int sprite_index;
        float duration;
    };

    std::vector<AnimationStepData> steps;
};

class AnimationPreview {
    const Animation* animation;
    size_t current_step;
    float current_duration;

  public:
    void set_animation(const Animation* anim);
    void update(float delta_time);
    int get_sprite_index();
};