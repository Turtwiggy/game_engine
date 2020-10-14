#pragma once

//your project headers
#include "engine/3d/camera.hpp"
#include "engine/renderer/shader.hpp"
#include "engine/geometry/triangle.hpp"

namespace fightingengine {

//Resources
//https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
//https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
//https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
//https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing

class RendererRayTraced
{
public:
    void init(int screen_width, int screen_height);
    void resize(Camera& c, int width, int height);

    [[nodiscard]] Shader& first_geometry_pass
    (
        const Camera& camera,
        int width,
        int height
    );

    void second_raytrace_pass
    (
        const Camera& camera,
        float   width,
        float   height,
        const std::vector<FETriangle>& triangles,
        float   timer,
        bool    force_refresh = false
    );

    void third_quad_pass();
};

} //namespace fightingengine




//#include "renderer.h"
//
//namespace fightingengine
//{
//    struct RenderData
//    {
//        //textures
//        unsigned int wood_texture;
//        unsigned int second_texture;
//    };
//    static RenderData s_Data;
//
//    //void Renderer::init_renderer(int screen_width, int screen_height)
//    //{
//    //    // load textures
//    //    // -------------
//    //    s_Data.wood_texture = TextureFromFile("BambooWall_1K_albedo.jpg", "assets/textures/Bamboo", false);
//    //    s_Data.second_texture = TextureFromFile("container.jpg", "assets/textures", false);
//
//    //    //GPU Info
//    //    printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
//    //    int params[1];
//    //    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
//    //    printf("GPU INFO: Max shader storage buffer bindings: %i \n", params[0]);
//    //    //GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
//    //    //GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
//    //    //GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
//    //    //GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
//    //    //GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
//    //    //GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
//    //    //GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
//    //    //GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
//    //    //GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96
//
//    //    // draw as wireframe
//    //    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//    //    //RenderCommand::init();
//    //    //RenderCommand::set_viewport(0, 0, screen_width, screen_height);
//    //}
//
//}
