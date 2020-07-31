#pragma once
#include "pch.h"
#include "Application.h"
#include "Util.h"

#ifndef NDEBUG
#include "DebugCallback.h"
#endif

void Application::init() {
#ifndef NDEBUG
    printf("DEBUG MODE\n");
#endif

    //          Initialize SDL              //
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    // SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    window = SDL_CreateWindow("AnimationEditor", 3870, 1000, window_size.x,
                              window_size.y,
                              SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
    SDL_assert_always(window);

    sdl_renderer = SDL_CreateRenderer(window, -1, 0);

    // Use OpenGL 3.3 core
    const char* glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

#ifndef NDEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    //          Create openGL context       //
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
               SDL_GetError());
        SDL_assert(false);
    }

    //          Initialize GLEW             //
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }

    //          OpenGL configuration        //
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    glViewport(0, 0, window_size.x, window_size.y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(handle_gl_debug_output, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
#endif

    //          Setup ImGui context         //
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load shaders
    default_shader =
        Shader("../src/shaders/default.vert", "../src/shaders/default.frag");

    sheet_shader =
        SheetShader("../src/shaders/sheet.vert", "../src/shaders/sheet.frag");

    // Init vertex buffer for triangle strip rendering
    struct {
        glm::vec2 pos, uv_coord;
    } vertices[4];

    vertices[0] = {{0.0f, 0.0f}, {0.0f, 1.0f}};
    vertices[1] = {{1.0f, 0.0f}, {1.0f, 1.0f}};
    vertices[2] = {{0.0f, 1.0f}, {0.0f, 0.0f}};
    vertices[3] = {{1.0f, 1.0f}, {1.0f, 0.0f}};

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]),
                          (void*)sizeof(glm::vec2));
    glEnableVertexAttribArray(1);

    running = true;
}

void Application::run() {
    last_frame_start = frame_start;
    frame_start = SDL_GetTicks();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    SDL_PumpEvents();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT ||
            (event.type == SDL_KEYDOWN &&
             event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) ||
            (event.type == SDL_WINDOWEVENT &&
             event.window.event == SDL_WINDOWEVENT_CLOSE &&
             event.window.windowID == SDL_GetWindowID(window))) {
            running = false;
        }
    }

    // Update mouse
    mouse.last_x = mouse.x;
    mouse.last_y = mouse.y;
    mouse.button_state = SDL_GetMouseState(&mouse.x, &mouse.y);

    { // Update gui
        using namespace ImGui;

        const ImVec2 window_pos = {-5.0, -5.0};
        SetNextWindowPos(window_pos);
        SetNextWindowSize(ImVec2(static_cast<float>(ui_size.x) - window_pos.x,
                                 static_cast<float>(ui_size.y) - window_pos.y));
        Begin("Controls", NULL,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        if (Button("Open...")) {
            open_file();
        }
        SameLine();
        if (Button("Save")) {
            save_file(false);
        }
        SameLine();
        if (Button("Save as...")) {
            save_file(true);
        }

        if (Checkbox("Preview animation", &show_preview)) {
            if (show_preview) {
                window_size.y = std::max(
                    sprite_sheet.dimensions.y + sprite_dimensions.y, ui_size.y);
                change_window_size();
            } else {
                window_size.y = std::max(sprite_sheet.dimensions.y, ui_size.y);
                change_window_size();
            }
        }

        PushItemWidth(100);
        if (DragInt2("Sprite dimensions", (int*)&sprite_dimensions, 1.0f, 1,
                     0)) {
            if (show_preview) {
                window_size.y = std::max(
                    sprite_sheet.dimensions.y + sprite_dimensions.y, ui_size.y);
                change_window_size();
            }
        }

        NewLine();
        Text("Animations");

        static const char* current_item_name;

        if (selected_anim_index < animations.size()) {
            current_item_name = animations[selected_anim_index].name;
        } else {
            current_item_name = "none";
        }

        if (BeginCombo("", current_item_name)) {
            for (size_t i = 0; i < animations.size(); ++i) {
                bool is_selected = (selected_anim_index == i);
                if (Selectable(animations[i].name, is_selected)) {
                    selected_anim_index = i;
                    preview.set_animation(&animations[selected_anim_index]);
                }
            }
            EndCombo();
        }
        SameLine();
        if (Button("Add")) {
            Animation new_anim;
            _itoa_s(static_cast<int>(animations.size()), new_anim.name, 10);
            animations.push_back(new_anim);
            selected_anim_index = animations.size() - 1;
        }
        SameLine();
        if (Button("Remove")) {
            animations.erase(animations.begin() + selected_anim_index);
        }
        SameLine();
        if (Button("Set name")) {
            OpenPopup("Set name.");
        }

        if (selected_anim_index < animations.size()) {
            auto& selected_anim = animations[selected_anim_index];

            SetNextItemWidth(300);
            if (BeginPopupModal("Set name.")) {
                static char new_name_buf[Animation::MAX_NAME_LENGTH];
                InputTextWithHint("Name", "New name", new_name_buf,
                                  Animation::MAX_NAME_LENGTH);
                if (Button("Set")) {
                    strcpy_s(selected_anim.name, new_name_buf);
                    new_name_buf[0] = '\0';
                    CloseCurrentPopup();
                }
                EndPopup();
            }

            Separator();

            char buf[32];
            for (size_t i = 0; i < selected_anim.steps.size(); ++i) {
                _itoa_s(static_cast<int>(i), buf, 10);
                PushID(buf);

                auto& step = selected_anim.steps[i];

                // TODO: display image

                int new_sprite_id = static_cast<int>(step.sprite_index);
                InputInt("Sprite id", &new_sprite_id, 1);
                new_sprite_id =
                    std::clamp(new_sprite_id, 0, static_cast<int>(num_sprites));
                step.sprite_index = static_cast<uint>(new_sprite_id);

                InputFloat("Duration", &step.duration, 1.0f, 0.0f, "% .2f");
                step.duration = std::clamp(step.duration, 0.0f, 1000.0f);

                if (Button("Remove")) {
                    selected_anim.steps.erase(selected_anim.steps.begin() + i);
                }
                PopID();
            }

            if (Button("Add step")) {
                selected_anim.steps.push_back({0, 0.0f});
            }
        }

        End();
    }

    // Update preview
    if (show_preview) {
        float delta_time = static_cast<float>(frame_start - last_frame_start) /
                           static_cast<float>(frame_delay);
        preview.update(delta_time);
    }

    // Rendering
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (sprite_sheet.id != 0) {
        // Render sprite sheet
        default_shader.use();
        projection = glm::ortho(0.0f, static_cast<float>(window_size.x),
                                static_cast<float>(window_size.y), 0.0f);
        default_shader.set_projection(projection);

        glm::vec2 render_position = {static_cast<float>(ui_size.x), 0.0f};
        default_shader.set_render_position(render_position);

        glBindTexture(GL_TEXTURE_2D, sprite_sheet.id);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArrays(GL_LINE_STRIP, 0, 4);

        // Render preview
        if (show_preview && selected_anim_index < animations.size()) {
            sheet_shader.use();
            sheet_shader.set_projection(projection);

            render_position.y += sprite_sheet.dimensions.y;
            sheet_shader.set_render_position(render_position);

            sheet_shader.set_sprite_dimensions(
                static_cast<glm::vec2>(sprite_dimensions));
            sheet_shader.set_sprite_index(
                static_cast<GLint>(preview.get_sprite_index()));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);

    // Wait for next frame
    u32 last_frame_time = SDL_GetTicks() - frame_start;
    if (frame_delay > last_frame_time)
        SDL_Delay(frame_delay - last_frame_time);
}

/*
    The binary file format for animations is as follows:
    u64             length of sprite sheet path (incl. /0)
    char[]          sprite sheet path
    u64             number of animations
    Animation[]     in basically the same format as the struct
 */

void Application::open_file() {
    // Get path
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    SDL_assert_always(SUCCEEDED(hr));
    IFileOpenDialog* pFileOpen;

    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                          IID_IFileOpenDialog,
                          reinterpret_cast<void**>(&pFileOpen));

    SDL_assert_always(SUCCEEDED(hr));

    COMDLG_FILTERSPEC file_type = {L".png, .anim", L"*.png; *.anim"};
    pFileOpen->SetFileTypes(1, &file_type);

    // Show the Open dialog box.
    hr = pFileOpen->Show(NULL);

    if (!SUCCEEDED(hr)) {
        return;
    }

    // Get the file name from the dialog box.
    IShellItem* pItem;
    hr = pFileOpen->GetResult(&pItem);

    SDL_assert_always(SUCCEEDED(hr));
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    size_t path_length = wcslen(pszFilePath) + 1;
    char* new_path = new char[path_length];

    wcstombs_s(nullptr, new_path, path_length, pszFilePath, path_length);

    CoTaskMemFree(pszFilePath);

    SDL_assert_always(SUCCEEDED(hr));

    pItem->Release();
    pFileOpen->Release();

    CoUninitialize();

    const char* extension = strrchr(new_path, '.');
    SDL_assert_always(extension);

    if (opened_path) {
        delete[] opened_path;
        opened_path = nullptr;
    }
    if (sprite_sheet_path) {
        delete[] sprite_sheet_path;
        sprite_sheet_path = nullptr;
    }
    animations.clear();

    if (strcmp(extension, ".png") == 0) {
        sprite_sheet_path = new char[path_length];
        strncpy_s(sprite_sheet_path, path_length, new_path, path_length);

        // Create new sheet from .png
        sprite_sheet.load_from_file(sprite_sheet_path);

        // Make a reasonable guess at the new sprite sheets sprite dimensions
        sprite_dimensions = glm::uvec2(greatest_common_divisor(
            sprite_sheet.dimensions.x, sprite_sheet.dimensions.y));
    } else {
        opened_path = new char[path_length];
        strncpy_s(opened_path, path_length, new_path, path_length);

        // Load animation from binary .anim file
        SDL_RWops* file = SDL_RWFromFile(opened_path, "rb");
        if (!file) {
            printf("[ERROR] Could not open file \"%s\"; %s", opened_path,
                   SDL_GetError());
            SDL_TriggerBreakpoint();
        }

        u64 sheet_path_length;
        SDL_RWread(file, &sheet_path_length, sizeof(u64), 1);

        sprite_sheet_path = new char[sheet_path_length];
        SDL_RWread(file, sprite_sheet_path, sizeof(char), sheet_path_length);

        sprite_sheet.load_from_file(sprite_sheet_path);

        SDL_RWread(file, value_ptr(sprite_dimensions), sizeof(glm::ivec2), 1);

        u64 num_animations;
        SDL_RWread(file, &num_animations, sizeof(u64), 1);

        animations.reserve(num_animations);

        for (size_t n_anim = 0; n_anim < num_animations; ++n_anim) {
            Animation anim;
            SDL_RWread(file, anim.name, sizeof(char),
                       Animation::MAX_NAME_LENGTH);

            u64 num_steps;
            SDL_RWread(file, &num_steps, sizeof(u64), 1);

            anim.steps.reserve(num_steps);

            for (size_t n_step = 0; n_step < num_steps; ++n_step) {
                Animation::AnimationStepData step;
                SDL_RWread(file, &step.sprite_index, sizeof(s64), 1);
                SDL_RWread(file, &step.duration, sizeof(float), 1);
                anim.steps.push_back(step);
            }
            animations.push_back(anim);
        }

        SDL_RWclose(file);

        // @CLEANUP: is it ok to only bind the texture here and never again?
        glBindTexture(GL_TEXTURE_2D, sprite_sheet.id);
    }

    num_sprites = (sprite_sheet.dimensions.x / sprite_dimensions.x) *
                  (sprite_sheet.dimensions.y / sprite_dimensions.y);

    window_size.x = sprite_sheet.dimensions.x + ui_size.x;

    if (show_preview) {
        window_size.y = std::max(
            sprite_sheet.dimensions.y + sprite_dimensions.y, ui_size.y);
    } else {
        window_size.y = std::max(sprite_sheet.dimensions.y, ui_size.y);
    }

    change_window_size();

    if (animations.size() > 0) {
        selected_anim_index = 0;
        preview.set_animation(&animations[0]);
    }
}

void Application::save_file(bool get_new_path) {
    if (get_new_path || opened_path == nullptr) {
        // Get a new path
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                                              COINIT_DISABLE_OLE1DDE);

        SDL_assert_always(SUCCEEDED(hr));
        IFileSaveDialog* pFileSave;

        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                              IID_IFileSaveDialog,
                              reinterpret_cast<void**>(&pFileSave));

        SDL_assert_always(SUCCEEDED(hr));

        COMDLG_FILTERSPEC file_type = {L".anim", L"*.anim"};
        pFileSave->SetFileTypes(1, &file_type);

        // Show the Open dialog box.
        hr = pFileSave->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr)) {
            IShellItem* pItem;
            hr = pFileSave->GetResult(&pItem);

            SDL_assert_always(SUCCEEDED(hr));
            PWSTR pszFilePath;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

            // Copy path to opened_path
            if (opened_path) {
                delete[] opened_path;
            }

            size_t length = wcslen(pszFilePath) + 1;
            opened_path = new char[length];

            wcstombs_s(nullptr, opened_path, length, pszFilePath, length);

            CoTaskMemFree(pszFilePath);

            // Display the file name to the user.
            SDL_assert_always(SUCCEEDED(hr));

            pItem->Release();
            pFileSave->Release();

            CoUninitialize();
        }
    }

    // Write file
    SDL_RWops* file = SDL_RWFromFile(opened_path, "wb");
    if (!file) {
        printf("[ERROR] Could not open file \"%s\"; %s", opened_path,
               SDL_GetError());
        SDL_TriggerBreakpoint();
    }

    u64 sheet_path_length = strnlen_s(sprite_sheet_path, 256) + 1;
    SDL_RWwrite(file, &sheet_path_length, sizeof(u64), 1);

    SDL_RWwrite(file, sprite_sheet_path, sizeof(char), sheet_path_length);

    SDL_RWwrite(file, value_ptr(sprite_dimensions), sizeof(glm::ivec2), 1);

    u64 num_animations = animations.size();
    SDL_RWwrite(file, &num_animations, sizeof(u64), 1);

    for (auto& anim : animations) {
        SDL_RWwrite(file, anim.name, sizeof(char), Animation::MAX_NAME_LENGTH);

        u64 num_steps = anim.steps.size();
        SDL_RWwrite(file, &num_steps, sizeof(u64), 1);

        for (auto& step : anim.steps) {
            SDL_RWwrite(file, &step.sprite_index, sizeof(s64), 1);
            SDL_RWwrite(file, &step.duration, sizeof(float), 1);
        }
    }

    SDL_RWclose(file);
}

void Application::change_window_size() {
    SDL_SetWindowSize(window, window_size.x, window_size.y);
    glViewport(0, 0, window_size.x, window_size.y);
}