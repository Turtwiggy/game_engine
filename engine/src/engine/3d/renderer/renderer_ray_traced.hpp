#pragma once

// your project headers
#include "engine/3d/camera/camera.hpp"
#include "engine/graphics/shader.hpp"
#include "engine/graphics/triangle.hpp"

namespace fightingengine {

// Resources
// https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
// https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
// https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
// https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing

class RendererRayTraced
{
public:
  RendererRayTraced(int screen_width, int screen_height);

  [[nodiscard]] Shader& first_geometry_pass(const Camera& camera,
                                            int width,
                                            int height);

  void second_raytrace_pass(const Camera& camera,
                            float width,
                            float height,
                            const std::vector<Triangle>& triangles,
                            float timer,
                            bool force_refresh = false);

  void third_quad_pass();
};

} // namespace fightingengine

// how to get GPU info from OpenGL
// -------------------------------
//    //GPU Info
//    printf("OpenGL version supported by this platform (%s): \n",
//    glGetString(GL_VERSION)); int params[1];
//    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
//    printf("GPU INFO: Max shader storage buffer bindings: %i \n", params[0]);
//    //GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
//    //GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
//    //GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
//    //GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
//    //GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
//    //GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
//    //GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
//    //GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
//    //GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96
