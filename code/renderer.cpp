#include "renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assert.h>
#include <stdio.h>

#include "defines.h"
#include "game.h"
#include "platform.h"

GLFWwindow *window;
i32 window_width = 920;
i32 window_height = 680;

struct Shader
{
    u32 id;
};

Shader default_shader;

struct Vertex
{
    V2 position;
    V3 color;
};

struct DrawCall
{
    u32 offset;
};

u32 vertex_gpu_buffer;
u32 vertex_vao;

u32 vertex_count;
Vertex vertex_buffer[1024];

u32 draw_call_count;
DrawCall draw_call_buffer[1024];

struct RenderData
{
    Mat4 projection;
    Mat4 view;
};

RenderData render_data = {};
u32 uniform_buffer;

u32 debug_ray_count;
DebugRay debug_rays[64];


void DrawRect(V2 botleft, V2 size, V3 color);
void DrawQuad(V2 p0, V2 p1, V2 p2, V2 p3, V3 color);

// Keys ...

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

f32 GetTime()
{
    return glfwGetTime();
}

// Callacks
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

// Resources
//

Shader LoadShader(const char* vertex_file, const char* frag_file)
{
    char info_log[512];
    i32 status;

    TempMemory temp_region = ScratchAllocate();

    u8 *vertex_code = ReadFile(vertex_file, temp_region.arena).memory;
    assert(vertex_code);
    u32 vertex_prog = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_prog, 1, (char **)  &vertex_code, NULL);
    glCompileShader(vertex_prog);
    glGetShaderiv(vertex_prog, GL_COMPILE_STATUS, &status);
    if (!status) 
    {
        glGetShaderInfoLog(vertex_prog, 512, NULL, info_log);
        printf("Error compiling vertex shader (%s): %s", vertex_file, info_log);
        assert(0);
    }

    u8 *fragment_code = ReadFile(frag_file, temp_region.arena).memory;
    assert(fragment_code);
    u32 frag_prog = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_prog, 1, (char **) &fragment_code, NULL);
    glCompileShader(frag_prog);
    glGetShaderiv(frag_prog, GL_COMPILE_STATUS, &status);
    if (!status) 
    {
        glGetShaderInfoLog(frag_prog, 512, NULL, info_log);
        printf("Error compiling fragment shader (%s): %s", frag_file, info_log);
        assert(0);
    }

    Shader shader;
    shader.id = glCreateProgram();
    glAttachShader(shader.id, vertex_prog);
    glAttachShader(shader.id, frag_prog);
    glLinkProgram(shader.id);
    glGetProgramiv(shader.id, GL_LINK_STATUS, &status);

    if (!status) 
    {
        glGetProgramInfoLog(shader.id, 512, NULL, info_log);
        printf("Error linking shader: %s", info_log);
        assert(0);
    }

    glDeleteShader(vertex_prog);
    glDeleteShader(frag_prog);

    EndTempRegion(temp_region);

    return shader;
}

// Api lifecycle
//

void InitializeRenderer()
{
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

    glDisable(GL_CULL_FACE);

    default_shader = LoadShader("shader/default.vert", "shader/default.frag");

    glGenBuffers(1, &uniform_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RenderData), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniform_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenVertexArrays(1, &vertex_vao);
    glBindVertexArray(vertex_vao);

    glGenBuffers(1, &vertex_gpu_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_gpu_buffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*) offsetof(Vertex, color));

    glBindVertexArray(0);
}

void ShutdownRenderer()
{
    glfwTerminate();
}

bool IsWindowOpen()
{
    return !glfwWindowShouldClose(window);
}

void DrawFrame()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    vertex_count = 0;
    draw_call_count = 0;

    f32 tilesize = 32;

    // Draw map
    for (i32 x = 0; x < level.width; ++x)
    {
        for (i32 y = 0; y < level.height; ++y)
        {
            if (level.tiles[x + y * level.width])
            {
                DrawRect(v2(x * tilesize , y * tilesize), v2(tilesize), v3(0.2, 0.2, 0.9));
            }
        }
    }

    // Draw player
    DrawRect(v2(player.position.x * tilesize, player.position.y * tilesize), v2(tilesize), v3(0.9, 0.2, 0.2));

    // Draw debug rays
    DebugRay *ray = debug_rays;
    for (u32 i = 0; i < debug_ray_count; ++i, ++ray)
    {
        V2 dir = Norm(ray->p0 - ray->p1);
        V2 left = v2(dir.y, -dir.x);
        f32 thickness = 2;

        DrawQuad(ray->p0 * tilesize - left * thickness, 
                 ray->p0 * tilesize + left * thickness, 
                 ray->p1 * tilesize - left * thickness, 
                 ray->p1 * tilesize + left * thickness, v3(0, 1, 0));
    }

    render_data.projection = Ortho(-window_width / 2.0f, window_width / 2.0f, -window_height / 2.0f, window_height / 2.0f, 0.1, 100);
    render_data.view = LookAt(v3(0, 0, 50), v3(0, 0, 0), v3(0, 1, 0));

    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RenderData), &render_data);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_gpu_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertex_buffer, GL_DYNAMIC_DRAW);

    glViewport(0, 0, window_width, window_height);
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vertex_vao);
    glUseProgram(default_shader.id);

    DrawCall *draw = draw_call_buffer;
    for (u32 i = 0; i < draw_call_count; ++i, ++draw)
    {
        glDrawArrays(GL_TRIANGLE_STRIP, draw->offset, 4);
    }

    for (u32 key = 0; key < Key_Count; ++key)
    {
        prev_key_states[key] = IsKeyDown((Key) key);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

// Drawing
//

static inline Vertex *AllocVertex()
{
    assert(vertex_count < 1024);
    return &vertex_buffer[vertex_count++];
}

static inline DrawCall *StartDrawCall()
{
    assert(draw_call_count < 1024);
    DrawCall *draw = &draw_call_buffer[draw_call_count++];
    draw->offset = vertex_count;
    return draw;
}

void DrawQuad(V2 p0, V2 p1, V2 p2, V2 p3, V3 color)
{
    DrawCall *draw = StartDrawCall();

    Vertex *vert0 = AllocVertex();
    vert0->position = p0;
    vert0->color = color;

    Vertex *vert1 = AllocVertex();
    vert1->position = p1;
    vert1->color = color;

    Vertex *vert2 = AllocVertex();
    vert2->position = p2;
    vert2->color = color;

    Vertex *vert3 = AllocVertex();
    vert3->position = p3;
    vert3->color = color;
}

void DrawRect(V2 botleft, V2 size, V3 color)
{
    DrawCall *draw = StartDrawCall();

    Vertex *vert0 = AllocVertex();
    vert0->position = botleft;
    vert0->color = color;

    Vertex *vert1 = AllocVertex();
    vert1->position = v2(botleft.x + size.x, botleft.y);
    vert1->color = color;

    Vertex *vert2 = AllocVertex();
    vert2->position = v2(botleft.x, botleft.y + size.y);
    vert2->color = color;

    Vertex *vert3 = AllocVertex();
    vert3->position = v2(botleft.x + size.x, botleft.y + size.y);
    vert3->color = color;
}
