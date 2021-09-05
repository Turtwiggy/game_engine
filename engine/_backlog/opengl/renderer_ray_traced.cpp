
// // header
// #include "engine/3d/renderer/renderer_ray_traced.hpp"

// // other library headers
// #include <glm/glm.hpp>
// #include <glm/gtx/transform.hpp>
// #include <glm/vec4.hpp>

// #include "GL/glew.h"

// // your project headers
// #include "engine/graphics/render_command.hpp"
// #include "engine/graphics/shader.hpp"
// #include "engine/graphics/util/opengl_util.hpp"
// #include "engine/mesh/primitives.hpp"

// namespace engine {

// // Specific to the RayTracer's Shader
// struct ComputeShaderVertex
// {
//   glm::vec4 pos;
//   glm::vec4 nml;
//   glm::vec4 tex;
//   glm::vec4 colour;
// };
// struct ComputeShaderTriangle
// {
//   ComputeShaderVertex p0;
//   ComputeShaderVertex p1;
//   ComputeShaderVertex p2;
// };

// struct RayTracedData
// {
//   // Geometry pass
//   unsigned int g_buffer;
//   unsigned int g_position, g_normal, g_albedo_spec;
//   Shader geometry_shader;

//   // RayTracing
//   unsigned int ray_fbo;
//   unsigned int out_texture;
//   Shader compute_shader;
//   unsigned int compute_shader_workgroup_x;
//   unsigned int compute_shader_workgroup_y;
//   int compute_normal_binding;
//   int compute_out_tex_binding;
//   Shader quad_shader;

//   bool refresh_ssbo = false;
//   unsigned int ssbo;
//   unsigned int ssbo_binding;

//   size_t max_triangles = 100;
//   size_t set_triangles = 0;
//   std::vector<ComputeShaderTriangle> triangles;

//   primitives::Plane plane;

//   uint32_t draw_calls = 0;
// };
// static RayTracedData s_Data;

// RendererRayTraced::RendererRayTraced(int screen_width, int screen_height)
// {
//   // Configure OpenGL
//   RenderCommand::init();
//   RenderCommand::set_viewport(0, 0, screen_width, screen_height);

//   // A quad shader to render the full-screen quad VAO with the framebuffer as
//   // texture
//   Shader quad_shader = Shader()
//                          .attach_shader("assets/shaders/raytraced/example.vert", OpenGLShaderTypes::VERTEX)
//                          .attach_shader("assets/shaders/raytraced/example.frag", OpenGLShaderTypes::FRAGMENT)
//                          .build_program();
//   quad_shader.bind();
//   quad_shader.set_int("tex", 0);
//   s_Data.quad_shader = quad_shader;
//   quad_shader.unbind();

//   Shader geometry_shader = Shader()
//                              .attach_shader("assets/shaders/raytraced/geometry.vert", OpenGLShaderTypes::VERTEX)
//                              .attach_shader("assets/shaders/raytraced/geometry.frag", OpenGLShaderTypes::FRAGMENT)
//                              .build_program();
//   s_Data.geometry_shader = geometry_shader;

//   // configure g-buffer for intial render pass
//   unsigned int gBuffer;
//   glGenFramebuffers(1, &gBuffer);
//   glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
//   unsigned int gPosition, gNormal, gAlbedoSpec;
//   {
//     // position color buffer
//     glGenTextures(1, &gPosition);
//     glBindTexture(GL_TEXTURE_2D, gPosition);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
//     // normal color buffer
//     glGenTextures(1, &gNormal);
//     glBindTexture(GL_TEXTURE_2D, gNormal);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
//     // color + specular color buffer
//     glGenTextures(1, &gAlbedoSpec);
//     glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
//     // tell OpenGL which color attachments we'll use (of this framebuffer) for
//     // rendering
//     unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
//     glDrawBuffers(3, attachments);
//     // create and attach depth buffer (renderbuffer)
//     unsigned int rboDepth;
//     glGenRenderbuffers(1, &rboDepth);
//     glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
//     // finally check if framebuffer is complete
//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//       std::cout << "Framebuffer not complete!" << std::endl;
//   }
//   s_Data.g_buffer = gBuffer;
//   s_Data.g_position = gPosition;
//   s_Data.g_normal = gNormal;
//   s_Data.g_albedo_spec = gAlbedoSpec;
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//   glBindRenderbuffer(GL_RENDERBUFFER, 0);

//   // Ray tracing texture that compute shader writes to
//   unsigned int rayFBO;
//   glGenFramebuffers(1, &rayFBO);
//   glBindFramebuffer(GL_FRAMEBUFFER, rayFBO);
//   unsigned int rayTexture;
//   {
//     // position color buffer
//     glGenTextures(1, &rayTexture);
//     glBindTexture(GL_TEXTURE_2D, rayTexture);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rayTexture, 0);
//     // tell OpenGL which color attachments we'll use (of this framebuffer) for
//     // rendering
//     unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
//     glDrawBuffers(1, attachments);
//     // finally check if framebuffer is complete
//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//       std::cout << "(Raytracer) ERROR: Framebuffer not complete!" << std::endl;
//   }
//   s_Data.ray_fbo = rayFBO;
//   s_Data.out_texture = rayTexture;
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);

//   // Ray tracing compute shader
//   Shader compute_shader =
//     Shader()
//       .attach_shader("assets/shaders/raytraced/compute/random.glsl", OpenGLShaderTypes::COMPUTE)
//       .attach_shader("assets/shaders/raytraced/compute/raytraced.glsl", OpenGLShaderTypes::COMPUTE)
//       .build_program();
//   compute_shader.bind();
//   CHECK_OPENGL_ERROR(12);

//   int workgroup_size[3];
//   glGetProgramiv(compute_shader.ID, GL_COMPUTE_WORK_GROUP_SIZE, workgroup_size);
//   unsigned int work_group_size_x = workgroup_size[0];
//   unsigned int work_group_size_y = workgroup_size[1];
//   unsigned int work_group_size_z = workgroup_size[2];
//   // link data
//   s_Data.compute_shader = compute_shader;
//   s_Data.compute_shader_workgroup_x = work_group_size_x;
//   s_Data.compute_shader_workgroup_y = work_group_size_y;
//   s_Data.compute_out_tex_binding = compute_shader.get_uniform_binding_location("outTexture");
//   // printf("(Raytracer) outtexbinding: %i \n",
//   // s_Data.compute_out_tex_binding);
//   s_Data.compute_normal_binding = compute_shader.get_uniform_binding_location("normalTexture");
//   // printf("(Raytracer) compute_normal_binding: %i \n",
//   // s_Data.compute_normal_binding);
//   s_Data.ssbo_binding = compute_shader.get_compute_buffer_binding_location("bufferData");

//   // Data to bind to GPU
//   // -------------------
//   // std::vector<ComputeShaderTriangle> triangles;
//   // triangles.resize(s_Data.max_triangles);
//   // printf("Size of init triangles: %i \n", sizeof(triangles));
//   /*glm::vec4 data = glm::vec4(1.0, 0.0, 0.0, 1.0);*/

//   // Ray tracing SSBO
//   // ----------------
//   printf("(Raytracer) ssbo bind slot: %i \n", s_Data.ssbo_binding);

//   GLuint SSBO;

//   glGenBuffers(1, &SSBO);
//   CHECK_OPENGL_ERROR(0);

//   glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
//   CHECK_OPENGL_ERROR(1)

//   // initialize empty ssbo
//   // glBufferData(GL_SHADER_STORAGE_BUFFER, s_Data.data.size() *
//   // sizeof(glm::vec4), &s_Data.data[0], GL_DYNAMIC_DRAW);
//   glBufferData(GL_SHADER_STORAGE_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
//   CHECK_OPENGL_ERROR(2);
//   // s_Data.refresh_ssbo = true;

//   glBindBufferBase(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo_binding, SSBO);
//   CHECK_OPENGL_ERROR(3);

//   glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//   CHECK_OPENGL_ERROR(4);

//   s_Data.ssbo = SSBO;
// }

// // 1. geometry pass: render scene's geometry/color data into gbuffer
// Shader&
// RendererRayTraced::first_geometry_pass(const Camera& camera, int width, int height)
// {
//   glm::mat4 view_projection = camera.get_view_projection_matrix();

//   glBindFramebuffer(GL_FRAMEBUFFER, s_Data.g_buffer);
//   CHECK_OPENGL_ERROR(13);
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//   // Setup geometry shader
//   {
//     s_Data.geometry_shader.bind();
//     s_Data.geometry_shader.set_mat4("view_projection", view_projection);
//     // either this or textures
//     // s_Data.geometry_shader.setVec3("diffuse", glm::vec3(1.0f, 0.0f, 0.0f));
//     s_Data.geometry_shader.set_float("specular", 1.0f);
//   }

//   return s_Data.geometry_shader;
// }

// // 2. Lighting pass: Raytracing
// void
// RendererRayTraced::second_raytrace_pass(const Camera& camera,
//                                         float width,
//                                         float height,
//                                         const std::vector<Triangle>& triangles,
//                                         float timer,
//                                         bool force_refresh)
// {
//   // Update scene's triangle description
//   {
//     if (s_Data.refresh_ssbo) {
//       std::vector<Triangle> triangles_in_scene = triangles;
//       size_t triangles_in_scene_size = triangles_in_scene.size();

//       // Check max triangles
//       if (triangles_in_scene_size > s_Data.max_triangles) {
//         printf("too many triangles! handle this scenario");
//         return;
//       }

//       // Check ssbo size
//       if (s_Data.set_triangles != triangles_in_scene_size) {
//         printf("Updating SSBO triangles with: %zi triangles \n", triangles_in_scene_size);

//         // refresh triangle data
//         s_Data.triangles.clear();
//         s_Data.triangles.resize(triangles_in_scene_size);

//         // convert FGTriangle to ComputeShaderTriangle
//         for (int i = 0; i < triangles_in_scene_size; i++) {
//           ComputeShaderVertex v1;
//           v1.pos = glm::vec4(triangles_in_scene[i].p0.Position, 1.0f);
//           v1.nml = glm::vec4(triangles_in_scene[i].p0.Normal, 1.0f);
//           v1.tex = glm::vec4(triangles_in_scene[i].p0.TexCoords, 1.0f, 1.0f);
//           v1.colour = triangles_in_scene[i].p0.Colour.colour;

//           ComputeShaderVertex v2;
//           v2.pos = glm::vec4(triangles_in_scene[i].p1.Position, 1.0f);
//           v2.nml = glm::vec4(triangles_in_scene[i].p1.Normal, 1.0f);
//           v2.tex = glm::vec4(triangles_in_scene[i].p1.TexCoords, 1.0f, 1.0f);
//           v2.colour = triangles_in_scene[i].p1.Colour.colour;

//           ComputeShaderVertex v3;
//           v3.pos = glm::vec4(triangles_in_scene[i].p2.Position, 1.0f);
//           v3.nml = glm::vec4(triangles_in_scene[i].p2.Normal, 1.0f);
//           v3.tex = glm::vec4(triangles_in_scene[i].p2.TexCoords, 1.0f, 1.0f);
//           v3.colour = triangles_in_scene[i].p2.Colour.colour;

//           s_Data.triangles[i].p0 = v1;
//           s_Data.triangles[i].p1 = v2;
//           s_Data.triangles[i].p2 = v3;
//         }

//         // upload data to ssbo when triangle size changes
//         // note: if the triangles vertices change, this wont update the
//         // ssbo currently
//         glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo);
//         glBufferData(GL_SHADER_STORAGE_BUFFER,
//                      triangles_in_scene_size * sizeof(ComputeShaderTriangle),
//                      &s_Data.triangles[0],
//                      GL_STATIC_DRAW);
//         // glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned
//         // int), (unsigned int)all_balls.size());
//         // glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int),
//         // all_balls.size() * sizeof(Ball), &(all_balls[0]));

//         s_Data.set_triangles = triangles_in_scene_size;
//       }

//       s_Data.refresh_ssbo = false;
//     }
//   }

//   glBindFramebuffer(GL_FRAMEBUFFER, s_Data.ray_fbo);
//   CHECK_OPENGL_ERROR(14);
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//   s_Data.compute_shader.bind();
//   CHECK_OPENGL_ERROR(5);

//   // Set viewing frustrum corner rays in shader
//   s_Data.compute_shader.set_vec3("eye", camera.Position);

//   glm::vec3 eye_ray;
//   eye_ray = camera.get_eye_ray(-1, -1);
//   s_Data.compute_shader.set_vec3("ray00", eye_ray);
//   eye_ray = camera.get_eye_ray(-1, 1);
//   s_Data.compute_shader.set_vec3("ray01", eye_ray);
//   eye_ray = camera.get_eye_ray(1, -1);
//   s_Data.compute_shader.set_vec3("ray10", eye_ray);
//   eye_ray = camera.get_eye_ray(1, 1);
//   s_Data.compute_shader.set_vec3("ray11", eye_ray);

//   s_Data.compute_shader.set_float("time", timer);

//   // Bind framebuffer texture as writable image in the shader.
//   glBindImageTexture(s_Data.compute_out_tex_binding, s_Data.out_texture, 0, false, 0, GL_WRITE_ONLY, GL_RGBA16F);
//   CHECK_OPENGL_ERROR(10);
//   glBindImageTexture(s_Data.compute_normal_binding, s_Data.g_normal, 0, false, 0, GL_READ_ONLY, GL_RGBA16F);
//   CHECK_OPENGL_ERROR(6);

//   s_Data.compute_shader.set_compute_buffer_bind_location("bufferData");
//   CHECK_OPENGL_ERROR(7);

//   glBindBufferBase(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo_binding, s_Data.ssbo);
//   CHECK_OPENGL_ERROR(9);

//   // set the ssbo size in a uniform
//   // s_Data.compute_shader.setInt("set_triangles", s_Data.set_triangles);

//   // Compute appropriate invocation dimension
//   int worksizeX = next_power_of_two(static_cast<unsigned int>(width));
//   int worksizeY = next_power_of_two(static_cast<unsigned int>(height));
//   if (s_Data.compute_shader_workgroup_x == 0 || s_Data.compute_shader_workgroup_y == 0) {
//     std::cout << "failed to load your compute shader!";
//     return;
//   }

//   /* Invoke the compute shader. */
//   // This function takes as argument the number of work groups in each of the
//   // three possible dimensions. The number of work groups is NOT the number of
//   // global work items in each dimension, but is divided by the work group size
//   // that the shader specified in that layout declaration.
//   glDispatchCompute(worksizeX / s_Data.compute_shader_workgroup_x, worksizeY / s_Data.compute_shader_workgroup_y, 1);

//   // Synchronize all writes to the framebuffer image
//   glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

//   // Reset bindings
//   glBindBufferBase(GL_SHADER_STORAGE_BUFFER, s_Data.ssbo_binding, 0);
//   glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
//   glUseProgram(0);
// }

// // 3. Normal drawing pass
// void
// RendererRayTraced::third_quad_pass()
// {
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//   s_Data.quad_shader.bind();
//   CHECK_OPENGL_ERROR(15);
//   glActiveTexture(GL_TEXTURE0);

//   // if (s_Data.is_c_held)
//   //{
//   glBindTexture(GL_TEXTURE_2D, s_Data.out_texture);
//   CHECK_OPENGL_ERROR(11);
//   //}
//   // else
//   // {
//   //     glBindTexture(GL_TEXTURE_2D, s_Data.g_albedo_spec);
//   // }

//   // Draw the plane
//   glBindVertexArray(s_Data.plane.vao);
//   if (s_Data.plane.Indices.size() > 0) {
//     glDrawElements(s_Data.plane.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES,
//                    s_Data.plane.Indices.size(),
//                    GL_UNSIGNED_INT,
//                    0);
//   } else {
//     glDrawArrays(s_Data.plane.topology == TOPOLOGY::TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES,
//                  0,
//                  s_Data.plane.Positions.size());
//   }
// }

// } // namespace engine
