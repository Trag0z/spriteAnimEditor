#pragma once
#include "pch.h"
#include "Types.h"
#include "Shader.h"
#include "Texture.h"

struct Animation {
    static const size_t MAX_NAME_LENGTH = 64;

    size_t num_steps = 0;
    char name[MAX_NAME_LENGTH] = "\n";

    struct AnimationStepData {
        int sprite_id;
        float duration;
    };

    std::list<AnimationStepData> steps;
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

    DefaultShader default_shader;
    SheetShader sheet_shader;

    glm::mat4 projection;

    GLuint vao, vbo;

    const glm::ivec2 ui_size = {300, 500};
    glm::ivec2 window_size = ui_size;

    // Animation Editor stuff
    size_t selected_anim_index;

    std::vector<Animation> animations;
    Texture sprite_sheet;

    glm::ivec2 sprite_dimensions;
    uint num_sprites;

    char* opened_path = nullptr;

    bool show_preview = false;

    void open_file();

  public:
    void init();
    void run();

    bool running = false;
};