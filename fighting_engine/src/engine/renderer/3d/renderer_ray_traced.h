//#pragma once
//
//#include "graphics/renderer.h"
//
//namespace fightingengine {
//
//    struct ComputeShaderVertex {
//        glm::vec4 pos;
//        glm::vec4 nml;
//        glm::vec4 tex;
//        glm::vec4 colour;
//    };
//    struct ComputeShaderTriangle {
//        ComputeShaderVertex p1;
//        ComputeShaderVertex p2;
//        ComputeShaderVertex p3;
//    };
//
//    //Resources
//    //https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
//    //https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
//    //https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
//    //https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing
//
//    class RendererRayTraced : public RendererImpl
//    {
//        void init(int screen_width, int screen_height);
//        void draw_pass(RenderDescriptor& desc);
//        void resize(int width, int height);
//    };
//
//}

////// renderQuad() renders a 1x1 XY quad in NDC
////// -----------------------------------------
////unsigned int quadVAO = 0;
////unsigned int quadVBO;
////void renderQuad()
////{
////    if (quadVAO == 0)
////    {
////        float quadVertices[] = {
////            // positions        // texture Coords
////            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
////            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
////             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
////             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
////        };
////        // setup plane VAO
////        glGenVertexArrays(1, &quadVAO);
////        glGenBuffers(1, &quadVBO);
////        glBindVertexArray(quadVAO);
////        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
////        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
////        glEnableVertexAttribArray(0);
////        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
////        glEnableVertexAttribArray(1);
////        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
////    }
////    glBindVertexArray(quadVAO);
////    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
////    glBindVertexArray(0);
////}


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
