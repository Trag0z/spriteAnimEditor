#pragma once
#include "pch.h"
#include "Animation.h"

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
    if (animation == nullptr || animation->steps.size() == 0) {
        return -1;
    }
    return animation->steps[current_step].sprite_index;
}
