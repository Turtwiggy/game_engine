// #pragma once

// // your project headers
// #include "engine/3d/camera/camera.hpp"
// #include "engine/graphics/shader.hpp"
// #include "engine/graphics/triangle.hpp"

// namespace engine {

// Resources
// https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-%28Part-I%29
// https://raytracing.github.io/books/RayTracingInOneWeekend.html#thevec3class
// https://github.com/LWJGL/lwjgl3-demos/blob/master/src/org/lwjgl/demo/opengl/raytracing/tutorial/Tutorial1.java#L482
// https://github.com/LWJGL/lwjgl3-demos/tree/master/src/org/lwjgl/demo/opengl/raytracing

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

// } // namespace engine

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

// std::vector<Sphere> create_world()
// {
//     return spheres;
// }