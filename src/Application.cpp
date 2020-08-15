#pragma once
#include "pch.h"
#include "Application.h"

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

    line_shader =
        LineShader("../src/shaders/line.vert", "../src/shaders/line.frag");

    // Init vertex buffer for triangle strip rendering
    struct {
        glm::vec2 pos, uv_coord;
    } sprite_vertices[4];

    sprite_vertices[0] = {{0.0f, 0.0f}, {0.0f, 1.0f}};
    sprite_vertices[1] = {{1.0f, 0.0f}, {1.0f, 1.0f}};
    sprite_vertices[2] = {{0.0f, 1.0f}, {0.0f, 0.0f}};
    sprite_vertices[3] = {{1.0f, 1.0f}, {1.0f, 0.0f}};

    glGenVertexArrays(1, &sprite_vao);
    glBindVertexArray(sprite_vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_vertices), sprite_vertices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(sprite_vertices[0]),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(sprite_vertices[0]),
                          (void*)sizeof(glm::vec2));
    glEnableVertexAttribArray(1);

    // Init vertex buffer for line loop rendering
    glm::vec2 line_vertices[4];

    line_vertices[0] = {0.0f, 0.0f};
    line_vertices[1] = {1.0f, 0.0f};
    line_vertices[2] = {1.0f, 1.0f};
    line_vertices[3] = {0.0f, 1.0f};

    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(line_vertices[0]),
                          (void*)0);
    glEnableVertexAttribArray(0);

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
                window_size.y = std::max(anim_sheet.sprite_sheet.dimensions.y +
                                             anim_sheet.sprite_dimensions.y,
                                         ui_size.y);
                change_window_size();
            } else {
                window_size.y =
                    std::max(anim_sheet.sprite_sheet.dimensions.y, ui_size.y);
                change_window_size();
            }
        }

        Checkbox("Lines between sprites", &show_lines);

        PushItemWidth(100);
        if (DragInt2("Sprite dimensions", (int*)&anim_sheet.sprite_dimensions,
                     1.0f, 1,
                     std::max(anim_sheet.sprite_sheet.dimensions.x,
                              anim_sheet.sprite_sheet.dimensions.y))) {
            window_size.x = std::max(anim_sheet.sprite_sheet.dimensions.x,
                                     anim_sheet.sprite_dimensions.x);
            if (show_preview) {
                window_size.y =
                    std::max(std::max(anim_sheet.sprite_sheet.dimensions.y,
                                      anim_sheet.sprite_dimensions.y) +
                                 anim_sheet.sprite_dimensions.y,
                             default_ui_size.y);
            } else {
                window_size.y =
                    std::max(std::max(anim_sheet.sprite_sheet.dimensions.y,
                                      anim_sheet.sprite_dimensions.y),
                             default_ui_size.y);
            }
            change_window_size();
        }

        NewLine();
        Text("Animations");

        static const char* current_item_name;

        if (selected_anim_index < anim_sheet.animations.size()) {
            current_item_name = anim_sheet.animations[selected_anim_index].name;
        } else {
            current_item_name = "none";
        }

        if (BeginCombo("", current_item_name)) {
            for (size_t i = 0; i < anim_sheet.animations.size(); ++i) {
                bool is_selected = (selected_anim_index == i);
                if (Selectable(anim_sheet.animations[i].name, is_selected)) {
                    selected_anim_index = i;
                    preview.set_animation(
                        &anim_sheet.animations[selected_anim_index]);
                }
            }
            EndCombo();
        }
        SameLine();
        if (Button("Add")) {
            Animation new_anim;
            _itoa_s(static_cast<int>(anim_sheet.animations.size()),
                    new_anim.name, 10);
            anim_sheet.animations.push_back(new_anim);
            selected_anim_index = anim_sheet.animations.size() - 1;
        }
        SameLine();
        if (Button("Remove")) {
            anim_sheet.animations.erase(anim_sheet.animations.begin() +
                                        selected_anim_index);
        }
        SameLine();
        if (Button("Set name")) {
            OpenPopup("Set name.");
        }

        if (selected_anim_index < anim_sheet.animations.size()) {
            auto& selected_anim = anim_sheet.animations[selected_anim_index];

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

                InputInt("Sprite id", &step.sprite_index, 1);
                step.sprite_index =
                    std::clamp(step.sprite_index, 0,
                               static_cast<int>(anim_sheet.num_sprites));

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
    glClear(GL_COLOR_BUFFER_BIT);

    if (anim_sheet.sprite_sheet.id != 0) {
        // Render sprite sheet
        default_shader.use();
        projection = glm::ortho(0.0f, static_cast<float>(window_size.x),
                                static_cast<float>(window_size.y), 0.0f);
        default_shader.set_projection(projection);

        glm::vec2 render_position = {static_cast<float>(ui_size.x), 0.0f};
        default_shader.set_render_position(render_position);

        glBindTexture(GL_TEXTURE_2D, anim_sheet.sprite_sheet.id);
        glBindVertexArray(sprite_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Render preview
        if (show_preview &&
            selected_anim_index < anim_sheet.animations.size()) {
            sheet_shader.use();
            sheet_shader.set_projection(projection);

            render_position.y += std::max(anim_sheet.sprite_sheet.dimensions.y,
                                          anim_sheet.sprite_dimensions.y);
            sheet_shader.set_render_position(render_position);

            sheet_shader.set_sprite_dimensions(
                static_cast<glm::vec2>(anim_sheet.sprite_dimensions));
            sheet_shader.set_sprite_index(
                static_cast<GLint>(preview.get_sprite_index()));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        // Render lines
        if (show_lines) {
            line_shader.use();
            line_shader.set_projection(projection);
            line_shader.set_sprite_dimensions(
                static_cast<glm::vec2>(anim_sheet.sprite_dimensions));
            line_shader.set_color(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

            glBindVertexArray(line_vao);

            for (glm::i32 i = 0;
                 i < static_cast<glm::i32>(anim_sheet.num_sprites); ++i) {
                glm::i32 sprites_per_row =
                    anim_sheet.sprite_sheet.dimensions.x /
                    anim_sheet.sprite_dimensions.x;

                glm::vec2 position;
                position.x = static_cast<float>(
                    ui_size.x +
                    i % sprites_per_row * anim_sheet.sprite_dimensions.x);
                position.y = static_cast<float>(i / sprites_per_row *
                                                anim_sheet.sprite_dimensions.y);

                line_shader.set_render_position(position);
                glDrawArrays(GL_LINE_LOOP, 0, 4);
            }
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

    if (strcmp(extension, ".png") == 0) {
        anim_sheet.create_new_from_png(new_path);
    } else {
        opened_path = new char[path_length];
        strncpy_s(opened_path, path_length, new_path, path_length);

        anim_sheet.load_from_text_file(new_path);

        // @CLEANUP: is it ok to only bind the texture here and never again?
        glBindTexture(GL_TEXTURE_2D, anim_sheet.sprite_sheet.id);
    }

    window_size.x = anim_sheet.sprite_sheet.dimensions.x + ui_size.x;

    if (show_preview) {
        window_size.y = std::max(anim_sheet.sprite_sheet.dimensions.y +
                                     anim_sheet.sprite_dimensions.y,
                                 ui_size.y);
    } else {
        window_size.y =
            std::max(anim_sheet.sprite_sheet.dimensions.y, ui_size.y);
    }

    change_window_size();

    if (anim_sheet.animations.size() > 0) {
        selected_anim_index = 0;
        preview.set_animation(&anim_sheet.animations[0]);
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

    anim_sheet.save_to_text_file(opened_path);
}

void Application::change_window_size() {
    SDL_SetWindowSize(window, window_size.x, window_size.y);
    glViewport(0, 0, window_size.x, window_size.y);

    ui_size.y = std::clamp(ui_size.y, default_ui_size.y, window_size.y);
}