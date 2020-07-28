#pragma once
#include "pch.h"
#include "Types.h"
#include "Shader.h"
#include "Texture.h"

struct Animation {
    size_t num_steps = 0;

    struct AnimationStepData {
        uint sprite_id;
        uint duration;
    };

    std::vector<AnimationStepData> anim_steps;
};

class Application {
    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    SDL_GLContext gl_context;

    u32 last_frame_start, frame_start;
    u32 frame_delay = 60 / 1000;

    struct {
        int x, y;
        int last_x, last_y;
        u32 button_state;
    } mouse;

    Shader sheet_shader;
    Shader default_shader;

    // Animation Editor stuff
    std::vector<Animation> animations;
    Texture sprite_sheet;

    glm::uvec2 sprite_dimensions;
    uint num_sprites;

    char* opened_path = nullptr;

    void open_file();

  public:
    void init();
    void run();

    bool running = false;
};