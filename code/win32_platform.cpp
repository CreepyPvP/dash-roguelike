#include "platform.h"

#include <assert.h>
#include <stdio.h>

#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define UFBX_ENABLE_TRIANGULATION
#include "ufbx.cpp"

#include "memory.cpp"
#include "game_math.cpp"
#include "opengl_renderer.cpp"

// #ifndef DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
// #endif

GLFWwindow *window;
i32 window_width = 960;
i32 window_height = 540;

V2 mouse_pos = {};
V2 prev_mouse_pos = {};
bool mouse_pos_updated = false;

GameAssets assets = {};

struct GameCode
{
    bool valid;
    HMODULE game_code_dll;
    FILETIME last_dll_modification;
    GameUpdateCall *GameUpdate;
    GameInitializeCall *GameInitialize;
};

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

void MouseCallback(GLFWwindow *window, f64 mouse_x, f64 mouse_y)
{
    prev_mouse_pos = mouse_pos;
    mouse_pos = v2(mouse_x, mouse_y);
    mouse_pos_updated = true;
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

// FBX LOADING. MOVE THIS SOMEWHERE ELSE

inline V2 ufbx_to_v2(ufbx_vec2 vec)
{
    return { (f32) vec.x, (f32) vec.y };
}

inline V3 ufbx_to_v3(ufbx_vec3 vec)
{
    return { (f32) vec.x, (f32) vec.y, (f32) vec.z };
}

Mesh LoadFBXMesh(ufbx_mesh *mesh)
{
    TempMemory temp_region = ScratchAllocate();

    // TODO: mesh->material_parts contains the mesh split by material. Maybe use that?
    u32 num_triangles = mesh->num_triangles;
    Vertex *vertices = PushArray(temp_region.arena, Vertex, num_triangles * 3);
    u32 num_vertices = 0;

    u32 num_tri_indices = mesh->max_face_triangles * 3;
    u32 *tri_indices = PushArray(temp_region.arena, u32, num_tri_indices);

    for (u32 face_id = 0; face_id < mesh->num_faces; ++face_id)
    {
        // TODO: If this does not work look here first :)
        ufbx_face face = mesh->faces.data[face_id];

        u32 num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);

        for (u32 i = 0; i < num_tris * 3; ++i) {
            u32 index = tri_indices[i];

            Vertex *v = vertices + num_vertices;
            v->position = ufbx_to_v3(ufbx_get_vertex_vec3(&mesh->vertex_position, index));
            v->normal = ufbx_to_v3(ufbx_get_vertex_vec3(&mesh->vertex_normal, index));
            v->uv = ufbx_to_v2(ufbx_get_vertex_vec2(&mesh->vertex_uv, index));
            v->color = v3(1);

            num_vertices++;
        }
    }

    assert(num_vertices == num_triangles * 3);

    ufbx_vertex_stream streams[1] = {
        { vertices, num_vertices, sizeof(Vertex) },
    };
    u32 num_indices = num_triangles * 3;
    u32 *indices = PushArray(temp_region.arena, u32, num_indices);

    num_vertices = ufbx_generate_indices(streams, 1, indices, num_indices, NULL, NULL);

    Mesh result = CreateMesh(vertices, num_vertices, indices, num_indices);

    EndTempRegion(temp_region);

    return result;
}

void LoadAllFBXMeshes()
{
    ufbx_load_opts opts = {}; 
    opts.target_axes = ufbx_axes_right_handed_z_up;
    opts.target_unit_meters = 1.0f;
    opts.target_camera_axes = ufbx_axes_right_handed_z_up;
    opts.target_light_axes = ufbx_axes_right_handed_z_up;
    opts.space_conversion = UFBX_SPACE_CONVERSION_ADJUST_TRANSFORMS;

    ufbx_error error; 
    ufbx_scene *scene = ufbx_load_file("assets/alien.fbx", &opts, &error);

    if (!scene) 
    {
        fprintf(stderr, "Failed to load: %s\n", error.description.data);
        assert(scene);
    }

    // for (i32 i = 0; i < scene->meshes.count; ++i)
    // {
    //     ufbx_mesh *mesh = scene->meshes.data[i];
    //     some_mesh = LoadFBXMesh(mesh);
    // }

    assets.alien = LoadFBXMesh(scene->meshes.data[0]);

    ufbx_free_scene(scene);
}

// 

FILETIME GetDLLWriteTime()
{
    WIN32_FIND_DATA file_data = {};
    FindFirstFile("game.dll", &file_data);
    return file_data.ftLastWriteTime;
}

GameCode LoadGameCode()
{
    GameCode result = {};
    bool copy = CopyFile("game.dll", "game_temp.dll", false);

    if (!copy)
    {
        i32 error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND)
        {
            printf("Failed to copy game code: File not found\n");
        }
        return result;
    }

    result.game_code_dll = LoadLibrary("game_temp.dll");
    if (result.game_code_dll)
    {
        result.last_dll_modification = GetDLLWriteTime();
        result.GameUpdate = (GameUpdateCall *) GetProcAddress(result.game_code_dll, "GameUpdate");
        result.GameInitialize = (GameInitializeCall *) GetProcAddress(result.game_code_dll, "GameInitialize");
        result.valid = result.GameUpdate && result.GameInitialize;
    }

    // TODO: Set fallback procs here

    return result;
}

void UnloadGameCode(GameCode *game_code)
{
    if (game_code->game_code_dll)
    {
        FreeLibrary(game_code->game_code_dll);
    }
    game_code->valid = false;
    // TODO: Set fallback procs here
}

i32 main()
{
    scratch = {};
    scratch.capacity = MegaByte(1);
    scratch.memory = (u8*) malloc(scratch.capacity);

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
    glfwSetCursorPosCallback(window, MouseCallback);
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
    LoadAllFBXMeshes();

    u64 game_memory_size = MegaByte(10);
    u8 *game_memory = (u8 *) malloc(game_memory_size);

    GameCode game_code = LoadGameCode();

    game_code.GameInitialize(game_memory, game_memory_size);

    f32 prev_time = glfwGetTime();
    u32 prev_key_states = 0;

    while (!glfwWindowShouldClose(window))
    {
        f32 time = glfwGetTime();
        f32 delta = time - prev_time;
        prev_time = time;

        FILETIME dll_write_time = GetDLLWriteTime();
        if (CompareFileTime(&game_code.last_dll_modification, &dll_write_time) == -1)
        {
            UnloadGameCode(&game_code);
            game_code = LoadGameCode();
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (!mouse_pos_updated)
        {
            prev_mouse_pos = mouse_pos;
        }
        else
        {
            mouse_pos_updated = false;
        }

        GameInput input = {};
        input.time = time;
        input.delta = delta;
        input.prev_key_states = prev_key_states;
        input.mouse_pos = mouse_pos;
        input.prev_mouse_pos = prev_mouse_pos;

        for (u32 i = 0; i < Key_Count; ++i)
        {
            if (glfwGetKey(window, key_mapping[i]) == GLFW_PRESS)
            {
                input.key_states |= (1 << i);
            }
        }
        prev_key_states = input.key_states;

        RenderData *render_data = game_code.GameUpdate(&input, &assets, game_memory);

        DrawFrame(render_data, window_width, window_height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}
