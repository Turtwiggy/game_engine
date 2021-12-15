
// // your header
// #include "engine/opengl/passes/shadowmapping.hpp"

// // other library files
// #include <GL/glew.h>

// // your project files
// #include "engine/opengl/framebuffer.hpp"
// #include "engine/opengl/render_command.hpp"

// namespace engine {

// ShadowmappingRenderpass::ShadowmappingRenderpass()
// {
//   // load shaders
//   ResourceManager::load_shader("assets/shaders/effects/",
//                                { "shadow_mapping_depth.vert", "shadow_mapping_depth.frag" },
//                                "SHADER_shadow_mapping_depth");

//   // shader config
//   shadowmap_depth_shader = ResourceManager::get_shader("SHADER_shadow_mapping_depth");

//   // Create depthbuffer fbo
//   create_shadowmap_depthbuffer(depthmap_fbo_, depthmap_tex, shadowmap_width_, shadowmap_height_);
// }

// void
// ShadowmappingRenderpass::create_shadowmap_depthbuffer(unsigned int& depth_map_fbo,
//                                                       unsigned int& depth_map_tex,
//                                                       int tex_width,
//                                                       int tex_height)
// {
//   glGenFramebuffers(1, &depth_map_fbo);
//   // create depth texture
//   glGenTextures(1, &depth_map_tex);
//   glBindTexture(GL_TEXTURE_2D, depth_map_tex);
//   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, tex_width, tex_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//   float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
//   glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
//   // attach depth texture as FBO's depth buffer
//   glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
//   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_tex, 0);
//   glDrawBuffer(GL_NONE);
//   glReadBuffer(GL_NONE);
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// glm::mat4
// ShadowmappingRenderpass::calculate_light_space_matrix(const glm::vec3& light_pos)
// {
//   float near_plane = 0.1f, far_plane = 20.0f;
//   glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
//   glm::mat4 light_view = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//   glm::mat4 light_space_matrix = light_projection * light_view;
//   return light_space_matrix;
// }

// void
// ShadowmappingRenderpass::bind(const glm::mat4& light_space_matrix)
// {
//   // render scene from light's point of view
//   shadowmap_depth_shader.bind();
//   shadowmap_depth_shader.set_mat4("light_space_matrix", light_space_matrix);

//   RenderCommand::set_viewport(0, 0, shadowmap_width_, shadowmap_height_);
//   Framebuffer::bind_fbo(depthmap_fbo_);

//   glClear(GL_DEPTH_BUFFER_BIT);
// }

// void
// ShadowmappingRenderpass::unbind()
// {
//   Framebuffer::default_fbo();
// }

// } // namespace engine