#pragma once

#include "defines.h"
#include "platform.h"

void InitializeRenderer();
void DrawFrame(RenderData *render_data, i32 window_width, i32 window_height);
void DoFbxTesting();

Mesh CreateMesh(Vertex *vertices, u32  vertex_count, u32 *indices, u32 index_count);
