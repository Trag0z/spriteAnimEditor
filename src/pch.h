#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <shobjidl.h>

// Both of these raise warnings, I don't know what to do but to ignore them
#pragma warning(push, 0)
#include <GL/glew.h>
#include <glm/ext.hpp>
#pragma warning(pop)

#include <glm/glm.hpp>

#include <sdl/SDL.h>
#include <sdl/SDL_image.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>
