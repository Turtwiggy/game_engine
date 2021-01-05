// #pragma once

// // your project headers
// #include "engine/3d/camera/camera.hpp"
// #include "engine/graphics/shader.hpp"
// #include "engine/graphics/triangle.hpp"

// namespace fightingengine {

// // Resources
// // https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
// // https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
// //
// https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
// // https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing

// class RendererRayTraced
// {
// public:
//   RendererRayTraced(int screen_width, int screen_height);

//   [[nodiscard]] Shader& first_geometry_pass(const Camera& camera, int width, int height);

//   void second_raytrace_pass(const Camera& camera,
//                             float width,
//                             float height,
//                             const std::vector<Triangle>& triangles,
//                             float timer,
//                             bool force_refresh = false);

//   void third_quad_pass();
// };

// } // namespace fightingengine

// // how to get GPU info from OpenGL
// // -------------------------------
// //    //GPU Info
// //    printf("OpenGL version supported by this platform (%s): \n",
// //    glGetString(GL_VERSION)); int params[1];
// //    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
// //    printf("GPU INFO: Max shader storage buffer bindings: %i \n", params[0]);
// //    //GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
// //    //GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
// //    //GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
// //    //GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
// //    //GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
// //    //GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
// //    //GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
// //    //GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
// //    //GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96

// // old rendering code
// // ------------------
// // void
// // render(RendererRayTraced& renderer, const Camera& cam, GameWindow& window, float timer)
// // {
// //   glm::ivec2 window_size = window.get_size();

// //   // First pass: render all objects
// //   // RenderCommand::set_clear_colour(glm::vec4(1.0, 0.0, 0.0, 1.0));
// //   Shader& shader = renderer.first_geometry_pass(cam, window_size.x, window_size.y);

// //   glm::mat4 model = glm::mat4(1.0f);
// //   model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
// //   model = glm::scale(model, glm::vec3(1.0f));
// //   shader.set_mat4("model", model);

// //   // model = glm::mat4(1.0f);
// //   // model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0));
// //   // model = glm::scale(model, glm::vec3(2.0f));
// //   // shader.setMat4("model", model);
// //   // cubes[1]->model->draw(shader, draw_calls);

// //   // Second pass: pass raytracer triangle information
// //   std::vector<Triangle> t;
// //   // t = cubes[0]->model->get_all_triangles_in_meshes();
// //   renderer.second_raytrace_pass(cam, static_cast<float>(window_size.x), static_cast<float>(window_size.y), t,
// timer);

// //   // Third pass: render scene information to quad
// //   renderer.third_quad_pass();
// // }

// // std::vector<Sphere> create_world()
// // {
// //     //ground sphere
// //     Sphere s4;
// //     s4.position = glm::vec3(0.0f, -100.5f, -1.0f);
// //     s4.radius = 100.0f;
// //     s4.mat.material_type = MATERIAL_DIFFUSE;
// //     s4.mat.albedo_colour = glm::vec3(0.8f, 0.8f, 0.0f);
// //     //main sphere
// //     Sphere s1;
// //     s1.position = glm::vec3(0.0f, 0.0f, -1.0f);
// //     s1.radius = 0.5f;
// //     s1.mat.material_type = MATERIAL_DIFFUSE;
// //     s1.mat.albedo_colour = glm::vec3(0.7f, 0.3f, 0.3f);
// //     //left sphere
// //     Sphere s2;
// //     s2.position = glm::vec3(-1.0f, 0.0f, -1.0f);
// //     s2.radius = 0.5f;
// //     s2.mat.material_type = MATERIAL_METAL;
// //     s2.mat.albedo_colour = glm::vec3(0.8f, 0.8f, 0.8f);
// //     s2.mat.metal_fuzz = 0.3f;
// //     //right sphere
// //     Sphere s3;
// //     s3.position = glm::vec3(1.0f, 0.0f, -1.0f);
// //     s3.radius = 0.5f;
// //     s3.mat.material_type = MATERIAL_METAL;
// //     s3.mat.albedo_colour = glm::vec3(0.8f, 0.6f, 0.2f);
// //     s3.mat.metal_fuzz = 0.8f;
// //     //World
// //     std::vector<Sphere> spheres;
// //     spheres.push_back(s1);
// //     spheres.push_back(s2);
// //     spheres.push_back(s3);
// //     spheres.push_back(s4);

// //     return spheres;
// // }