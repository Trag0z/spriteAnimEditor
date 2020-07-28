#pragma once
#include "pch.h"
#include "Application.h"

int main(int argc, char* argv[]) {
    // I don't want to use the parameters, but they are required for SDL_main to
    // correctly identify the entry point
    static_cast<int>(argc);
    static_cast<char**>(argv);

    Application app;
    app.init();
    while (app.running)
        app.run();
    return 0;
}

