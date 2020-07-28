#pragma once

#define CPU_RENDERING

#pragma warning(push, 0)

#include <array>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits.h>
#include <list>

#include <shobjidl.h>

#include <GL/glew.h>
#include <gl/glew.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sdl/SDL.h>
#include <sdl/SDL_image.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

#pragma warning(pop)

// #undef GL_CLIP_ORIGIN

// #define SDL_MAIN_HANDLED
// #include <ctype.h>
// #include <float.h>
// #include <locale.h>
// #include <malloc.h>
// #include <math.h>
// #include <memory>
// #include <stdarg.h>
// #include <stddef.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
// #include <wchar.h>
// #include <wctype.h>

// // Windows SDK
// #define _WIN32_WINNT 0x0501 // _WIN32_WINNT_WINXP
// #include <SDKDDKVer.h>

// // Windows API
// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>