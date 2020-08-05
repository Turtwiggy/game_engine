#pragma once

#include "graphics/renderer.h"

namespace fightinggame {

    struct ComputeShaderVertex {
        glm::vec4 pos;
        glm::vec4 nml;
        glm::vec4 tex;
        glm::vec4 colour;
    };
    struct ComputeShaderTriangle {
        ComputeShaderVertex p1;
        ComputeShaderVertex p2;
        ComputeShaderVertex p3;
    };

    struct RayTracedData {
        //Geometry pass
        unsigned int g_buffer;
        unsigned int  g_position, g_normal, g_albedo_spec;
        Shader geometry_shader;

        //RayTracing
        unsigned int ray_fbo;
        unsigned int out_texture;
        Shader compute_shader;
        unsigned int compute_shader_workgroup_x;
        unsigned int compute_shader_workgroup_y;
        int compute_normal_binding;
        int compute_out_tex_binding;
        Shader quad_shader;

        bool refresh_ssbo = false;
        unsigned int ssbo;
        unsigned int ssbo_binding;

        unsigned int max_triangles = 100;
        unsigned int set_triangles = 0;
        std::vector<ComputeShaderTriangle> triangles;

        //input for debugging
        bool is_c_held = false;

        uint32_t draw_calls = 0;
    };
    static RayTracedData s_Data;

    //Resources
    //https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
    //https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
    //https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
    //https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing

    class RendererRayTraced : public RendererImpl
    {
        void init(int screen_width, int screen_height);
        void draw_pass(draw_scene_desc& desc, const GameState& state);
        void resize(int width, int height);
    };

}
