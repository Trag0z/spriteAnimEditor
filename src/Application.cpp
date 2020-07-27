#pragma once
#include "pch.h"
#include "Application.h"
#include "DebugCallback.h"

void Application::init() {
#ifndef NDEBUG
    printf("DEBUG MODE\n");
#endif

    //          Initialize SDL              //
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    // SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    window = SDL_CreateWindow("procTree", 3840, 956, 1920, 1080,
                              SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS |
                                  SDL_WINDOW_OPENGL);
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

    glm::ivec2 window_size;
    SDL_GetWindowSize(window, &window_size.x, &window_size.y);

    glViewport(0, 0, window_size.x, window_size.y);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_MULTISAMPLE);

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

    //          Create shaders              //
    shaders.construction = load_and_compile_shader_from_file(
        "../src/shaders/construction.vert", "../src/shaders/construction.geom",
        nullptr);

    shaders.line = load_and_compile_shader_from_file(
        "../src/shaders/line.vert", nullptr, "../src/shaders/line.frag");

    shaders.render = load_and_compile_shader_from_file(
        "../src/shaders/render.vert", nullptr, "../src/shaders/render.frag");

    //          Setup buffers               //
    start_vbo =
        ArrayBuffer(GL_ARRAY_BUFFER, GL_STREAM_DRAW, sizeof(Vertex) * 3);

    const GLuint max_triangles =
        calculate_num_triangles(MAX_GEOMETRY_ITERATIONS);

    feedback_vbo[0] = ArrayBuffer(GL_ARRAY_BUFFER, GL_STREAM_COPY,
                                  sizeof(Vertex) * max_triangles * 3);
    feedback_vbo[1] = ArrayBuffer(GL_ARRAY_BUFFER, GL_STREAM_COPY,
                                  sizeof(Vertex) * max_triangles * 3);

    start_vao = VertexArray(start_vbo);
    feedback_vao[0] = VertexArray(feedback_vbo[0]);
    feedback_vao[1] = VertexArray(feedback_vbo[1]);

    //          Initial data                //
    Vertex* vertices = new Vertex[max_triangles * 3];

    float sin = sinf(glm::radians(120.0f));
    float cos = cosf(glm::radians(120.0f));

    vertices[0].position = {-sin, 0.0f, cos, 1.0f};
    vertices[1].position = {0.0f, 0.0f, 1.0f, 1.0f};
    vertices[2].position = {sin, 0.0f, cos, 1.0f};

    vertices[0].normal = {0.0f, 1.0f, 0.0f};
    vertices[1].normal = {0.0f, 1.0f, 0.0f};
    vertices[2].normal = {0.0f, 1.0f, 0.0f};

    vertices[0].length = 5.0f;
    vertices[1].length = 5.0f;
    vertices[2].length = 5.0f;

    start_vbo.write_data(sizeof(Vertex) * 3, vertices);

    //          First Geometry pass          //
    glUseProgram(shaders.construction);
    start_vao.bind();
    feedback_vbo[0].bind_as_feedback_target();

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEndTransformFeedback();
    glFlush();

    run_geometry_pass = false;

    num_triangles = calculate_num_triangles(++geometry_iteration);

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

        Begin("Controls", NULL, ImGuiWindowFlags_NoTitleBar);
        run_geometry_pass = Button("Run geometry pass");
        Text("Geometry passes: %u/%u", geometry_iteration,
             MAX_GEOMETRY_ITERATIONS);

        NewLine();
        Text("Right mouse button to turn the model.");

        NewLine();
        Checkbox("Render model", &render_model);
        Checkbox("Render wireframes", &render_wireframes);

        NewLine();
        DragFloat3("Light position", value_ptr(light_position), 0.2f);

        End();
    }

    if (run_geometry_pass && geometry_iteration < MAX_GEOMETRY_ITERATIONS) {
        ++geometry_iteration;
        read_buffer_index = geometry_iteration % 2;
        write_buffer_index = read_buffer_index ^ 1;

        glUseProgram(shaders.construction);

        feedback_vao[read_buffer_index].bind();
        feedback_vbo[write_buffer_index].bind_as_feedback_target();

        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_TRIANGLES, 0, num_triangles * 3);
        glEndTransformFeedback();
        glFlush();

        num_triangles = calculate_num_triangles(geometry_iteration);
    }

    // Render
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update camera
    if (mouse.button_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        const float sensitivity = 0.03f;
        object_rotation += (mouse.x - mouse.last_x) * sensitivity;
    }

    glm::mat4 projection =
        glm::perspective(glm::radians(100.0f), 1920.0f / 1200.0f, 0.1f, 100.0f);

    const glm::mat4 view =
        glm::lookAt(camera.pos, camera.target, {0.0f, 1.0f, 0.0f});

    glm::mat4 model =
        glm::rotate(glm::mat4(1.0f), object_rotation, {0.0f, 1.0f, 0.0f});

    if (render_model) {
        glUseProgram(shaders.render);

        GLuint projection_id =
            glGetUniformLocation(shaders.render, "projection");
        glUniformMatrix4fv(projection_id, 1, 0, value_ptr(projection));

        GLuint view_id = glGetUniformLocation(shaders.render, "view");
        glUniformMatrix4fv(view_id, 1, 0, value_ptr(view));

        GLuint model_id = glGetUniformLocation(shaders.render, "model");
        glUniformMatrix4fv(model_id, 1, 0, value_ptr(model));

        GLuint light_pos_id = glGetUniformLocation(shaders.render, "light_pos");
        glUniform3fv(light_pos_id, 1, value_ptr(light_position));

        GLuint camera_pos_id =
            glGetUniformLocation(shaders.render, "camera_pos");
        glUniform3fv(camera_pos_id, 1, value_ptr(camera.pos));

        feedback_vao[write_buffer_index].bind();
        glDrawArrays(GL_TRIANGLES, 0, num_triangles * 3);
    }

    // Debug rendering
    glUseProgram(shaders.line);

    GLuint projection_id = glGetUniformLocation(shaders.line, "projection");
    glUniformMatrix4fv(projection_id, 1, 0, value_ptr(projection));

    GLuint view_id = glGetUniformLocation(shaders.line, "view");
    glUniformMatrix4fv(view_id, 1, 0, value_ptr(view));

    GLuint model_id = glGetUniformLocation(shaders.line, "model");
    glUniformMatrix4fv(model_id, 1, 0, value_ptr(model));

    if (render_wireframes) {
        glDrawArrays(GL_LINE_STRIP, 0, num_triangles * 3);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);

    // Wait for next frame
    u32 last_frame_time = SDL_GetTicks() - frame_start;
    if (frame_delay > last_frame_time)
        SDL_Delay(frame_delay - last_frame_time);
}
