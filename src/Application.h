#pragma once
#include "pch.h"
#include "Types.h"
#include "Vertex.h"

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

    bool running = false;

  public:
    void init();
    void run();
};