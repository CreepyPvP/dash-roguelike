#include "platform.h"

#include <assert.h>
#include <stdio.h>

#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "renderer.h"

GLFWwindow *window;
i32 window_width = 920;
i32 window_height = 680;

// Keys ...
// 

char key_mapping[] = {
    'W',
    'A',
    'S',
    'D',
    'C',
    'R',
};

bool prev_key_states[Key_Count] = {};

bool IsKeyDown(Key key)
{
    // NOTE: GLFW_KEY_W is equal to 'W'. GLFW uses ascii uppercase letters!!!
    return glfwGetKey(window, key_mapping[key]) == GLFW_PRESS;
}

bool IsKeyJustDown(Key key)
{
    return !prev_key_states[key] && IsKeyDown(key);
}

// File utils...
//

FileRead ReadFile(const char *filename, Arena *arena)
{
    FileRead read = {};

    FILE *file = fopen(filename, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    u8 *buffer = PushBytes(arena, len + 1);
    fseek(file, 0, SEEK_SET);
    fread(buffer, len, 1, file);
    buffer[len] = 0;

    fclose(file);

    read.bytes_read = len;
    read.memory = buffer;

    return read;
}

// Window / Opengl stuff...
//

void ResizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    window_width = width;
    window_height = height;
}

void APIENTRY DebugOutput(GLenum source, 
                          GLenum type, 
                          u32 id, 
                          GLenum severity, 
                          GLsizei length, 
                          const char *message, 
                          const void *userParam)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
    {
        return; 
    }
    
    if (severity == GL_DEBUG_SEVERITY_LOW) 
    {
        return;
    }

    const char *source_str;
    const char *type_str;
    const char *severity_str;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             source_str = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source_str = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: source_str = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     source_str = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     source_str = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           source_str = "Other"; break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               type_str = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_str = "Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         type_str = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         type_str = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              type_str = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          type_str = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           type_str = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               type_str = "Other"; break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         severity_str = "high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severity_str = "medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          severity_str = "low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severity_str = "notification"; break;
    }

    printf("OPENGL (%s, Source: %s, Type: %s): %s", severity_str, source_str, type_str, message);
}

i32 main()
{
    scratch = InitializeArena();

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
    
    GLFWmonitor *monitor = NULL;
    // if (fullscreen)
    if (0)
    {
        monitor = glfwGetPrimaryMonitor();
    }

    window = glfwCreateWindow(window_width, window_height, "Game", monitor, NULL);

    assert(window);

    glfwSetFramebufferSizeCallback(window, ResizeCallback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);

    assert(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback(DebugOutput, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    InitializeRenderer();

    Arena game_memory = InitializeArena();

    HMODULE game_code = LoadLibrary("game.dll");
    assert(game_code);
    GameUpdateCall *game_update = (GameUpdateCall *) GetProcAddress(game_code, "GameUpdate");
    GameInitializeCall *game_initialize = (GameInitializeCall *) GetProcAddress(game_code, "GameInitialize");
    assert(game_update && game_initialize);

    game_initialize();

    f32 prev_time = glfwGetTime();
    u32 prev_key_states = 0;

    while (!glfwWindowShouldClose(window))
    {
        f32 time = glfwGetTime();
        f32 delta = time - prev_time;
        prev_time = time;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        GameInput input = {};
        input.time = time;
        input.delta = delta;
        input.prev_key_states = prev_key_states;
        for (u32 i = 0; i < Key_Count; ++i)
        {
            if (glfwGetKey(window, key_mapping[i]) == GLFW_PRESS)
            {
                input.key_states |= (1 << i);
            }
        }
        prev_key_states = input.key_states;

        RenderData *render_data = game_update(&input);

        DrawFrame(render_data, window_width, window_height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
