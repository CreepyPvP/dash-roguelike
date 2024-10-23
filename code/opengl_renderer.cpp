#include "renderer_backend.h"

#include <glad/glad.h>

#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include "defines.h"
#include "memory.h"
#include "platform.h"

struct Shader
{
    u32 id;
};

Shader default_shader;

u32 vertex_gpu_buffer;
u32 vertex_vao;

u32 uniform_buffer;

struct UniformBuffer
{
    Mat4 projection;
    Mat4 view;
};

UniformBuffer uniforms;


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

inline V2 ufbx_to_v2(ufbx_vec2 vec)
{
    return { (f32) vec.x, (f32) vec.y };
}

inline V3 ufbx_to_v3(ufbx_vec3 vec)
{
    return { (f32) vec.x, (f32) vec.y, (f32) vec.z };
}

void LoadMesh(ufbx_mesh *mesh)
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

    // TODO: Create vertex buffer here
    // create_vertex_buffer(vertices, num_vertices);
    // create_index_buffer(indices, num_indices);

    EndTempRegion(temp_region);
}

void DoFbxTesting()
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

    for (i32 i = 0; i < scene->meshes.count; ++i)
    {
        ufbx_mesh *mesh = scene->meshes.data[i];
        LoadMesh(mesh);
    }

    ufbx_free_scene(scene);
}

// Api lifecycle
//

void InitializeRenderer()
{
    glDisable(GL_CULL_FACE);

    default_shader = LoadShader("shader/default.vert", "shader/default.frag");

    glGenBuffers(1, &uniform_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBuffer), NULL, GL_STATIC_DRAW);
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

inline void MultiDrawCommand(MultiDraw *draw)
{
    glMultiDrawArrays(GL_TRIANGLE_STRIP, draw->offsets, draw->counts, draw->primitive_count);
}

inline void SingleDrawCommand(SingleDraw *draw)
{
    glDrawArrays(GL_TRIANGLE_STRIP, draw->offset, draw->count);
}

void DrawFrame(RenderData *render_data, i32 window_width, i32 window_height)
{
    f32 tilesize = 32;

    // Draw enemies
    // for (u32 i = 0; i < level.enemy_count; ++i)
    // {
    //     Enemy *enemy = &level.enemies[i];
    //     
    //     if (enemy->flags & ENEMY_DEAD)
    //     {
    //         continue;
    //     }
    //     
    //     DrawRect(v2(enemy->position.x * tilesize, enemy->position.y * tilesize), v2(tilesize), v3(0.9, 0.2, 0.2));
    // }

    // Draw debug rays
    // for (u32 i = 0; i < debug_ray_count; ++i)
    // {
    //     DebugRay *ray = &debug_rays[i];
    //     V2 dir = Norm(ray->p0 - ray->p1);
    //     V2 left = v2(dir.y, -dir.x);
    //     f32 thickness = 2;
    //
    //     DrawQuad(ray->p0 * tilesize - left * thickness, 
    //              ray->p0 * tilesize + left * thickness, 
    //              ray->p1 * tilesize - left * thickness, 
    //              ray->p1 * tilesize + left * thickness, v3(0, 1, 0));
    // }

    f32 aspect = (f32) window_width / (f32) window_height;
    f32 viewport_height = 540;
    f32 viewport_width = aspect * viewport_height;
    uniforms.projection = Ortho(0, viewport_width, viewport_height, 0, 0.1, 100);
    uniforms.view = LookAt(v3(0, 0, 50), v3(0, 0, 0), v3(0, 1, 0));

    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBuffer), &uniforms);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_gpu_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * render_data->vertex_count, render_data->vertex_buffer, GL_DYNAMIC_DRAW);

    glViewport(0, 0, window_width, window_height);
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vertex_vao);
    glUseProgram(default_shader.id);

    MultiDrawCommand(&render_data->level);
    MultiDrawCommand(&render_data->entities);
    MultiDrawCommand(&render_data->player);
    MultiDrawCommand(&render_data->debug);
}

// Drawing
//

// static inline Vertex *AllocVertex()
// {
//     assert(vertex_count < 1024);
//     return &vertex_buffer[vertex_count++];
// }

// static inline DrawCall *StartDrawCall()
// {
//     assert(draw_call_count < 1024);
//     DrawCall *draw = &draw_call_buffer[draw_call_count++];
//     draw->offset = vertex_count;
//     return draw;
// }

// void DrawQuad(V2 p0, V2 p1, V2 p2, V2 p3, V3 color)
// {
//     DrawCall *draw = StartDrawCall();
//
//     Vertex *vert0 = AllocVertex();
//     vert0->position = p0;
//     vert0->color = color;
//
//     Vertex *vert1 = AllocVertex();
//     vert1->position = p1;
//     vert1->color = color;
//
//     Vertex *vert2 = AllocVertex();
//     vert2->position = p2;
//     vert2->color = color;
//
//     Vertex *vert3 = AllocVertex();
//     vert3->position = p3;
//     vert3->color = color;
// }
//
// void DrawRect(V2 botleft, V2 size, V3 color)
// {
//     DrawCall *draw = StartDrawCall();
//
//     Vertex *vert0 = AllocVertex();
//     vert0->position = botleft;
//     vert0->color = color;
//
//     Vertex *vert1 = AllocVertex();
//     vert1->position = v2(botleft.x + size.x, botleft.y);
//     vert1->color = color;
//
//     Vertex *vert2 = AllocVertex();
//     vert2->position = v2(botleft.x, botleft.y + size.y);
//     vert2->color = color;
//
//     Vertex *vert3 = AllocVertex();
//     vert3->position = v2(botleft.x + size.x, botleft.y + size.y);
//     vert3->color = color;
// }
