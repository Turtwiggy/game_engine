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
