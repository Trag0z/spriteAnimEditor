#pragma once
#include "pch.h"
#include "Types.h"
#include "Shader.h"
#include "Texture.h"
#include "Animation.h"

class Application {
    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    SDL_GLContext gl_context;

    u32 last_frame_start, frame_start;
    u32 frame_delay = 1000 / 60;

    struct {
        int x, y;
        int last_x, last_y;
        u32 button_state;
    } mouse;

    Shader default_shader;
    SheetShader sheet_shader;

    glm::mat4 projection;

    GLuint vao, vbo;

    const glm::ivec2 ui_size = {300, 500};
    glm::ivec2 window_size = ui_size;

    // Animation Editor stuff
    size_t selected_anim_index;
    std::vector<Animation> animations;

    AnimationPreview preview;

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