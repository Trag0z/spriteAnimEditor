#pragma once
#include "pch.h"
#include "Application.h"

// #pragma warning(push, 0)

int main(int argc, char* argv[]) {
    Application app;
    app.init();
    app.run();
    return 0;
}

// #pragma warning(pop)