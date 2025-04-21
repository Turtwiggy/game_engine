#include "engine/opengl/framebuffer.hpp"
#include "pch.hpp"

#include "modules/core/renderer/fluidsim/helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/texture.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/fluidsim/components.hpp"
#include "modules/core/renderer/helpers.hpp"

namespace game2d {

const auto render_fullscreen_quad = [](entt::registry& r, const engine::Shader& shader, const glm::ivec2& size) {
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ri.renderer.reset_quad_vert_count();
  ri.renderer.begin_batch();

  engine::quad_renderer::RenderDescriptor desc;
  desc.pos_tl = { 0, 0 };
  desc.size = size;
  desc.yaw_pitch_roll_radians = { 0, 0, 0 };

  ri.renderer.draw_sprite(desc, shader);

  ri.renderer.end_batch();
  ri.renderer.flush(shader);
};

void
rebind_fluidsim(entt::registry& r, FluidSimData& data, int i)
{
  // dye, double buffer
  glActiveTexture(GL_TEXTURE0 + data.dye.info[0].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.dye.info[0].tex.tex_id.id);
  glActiveTexture(GL_TEXTURE0 + data.dye.info[1].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.dye.info[1].tex.tex_id.id);

  // velocity, double buffer
  glActiveTexture(GL_TEXTURE0 + data.velocity.info[0].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.velocity.info[0].tex.tex_id.id);
  glActiveTexture(GL_TEXTURE0 + data.velocity.info[1].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.velocity.info[1].tex.tex_id.id);

  // divergence
  glActiveTexture(GL_TEXTURE0 + data.divergence.info.tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.divergence.info.tex.tex_id.id);

  // pressure, double buffer
  glActiveTexture(GL_TEXTURE0 + data.pressure.info[0].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.pressure.info[0].tex.tex_id.id);
  glActiveTexture(GL_TEXTURE0 + data.pressure.info[1].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.pressure.info[1].tex.tex_id.id);
};

void
load_fluidsim(entt::registry& r, FluidSimData& data, int used_texture_units)
{
  data.splatProgram = engine::Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/fluidsim/splat.frag");
  data.advectProgram = engine::Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/fluidsim/advect.frag");
  data.divergenceProgram = engine::Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/fluidsim/divergence.frag");
  data.pressureProgram = engine::Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/fluidsim/pressure.frag");
  data.gradientSubtractProgram =
    engine::Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/fluidsim/gradient_sub.frag");

  const auto sim_wh = glm::ivec2{ data.config_sim_resolution, data.config_sim_resolution };
  const auto dye_wh = glm::ivec2{ data.config_dye_resolution, data.config_dye_resolution };

  const auto create_tex = [&used_texture_units](FboAndTexInfo& out, glm::ivec2 wh) {
    DoubleBufferInfo info;

    // assign a texture unit
    Texture tex;
    tex.tex_unit = used_texture_units;

    // generate a texture and attach to fbo
    const engine::FboResult res = engine::new_texture_to_fbo(tex.tex_unit.unit, wh);
    tex.tex_id.id = res.out_tex_ids[0];

    out.tex = tex;
    out.fbo_id = res.out_fbo_id;

    used_texture_units++; // increment after
  };

  // dye, double buffer
  create_tex(data.dye.info[0], dye_wh);
  create_tex(data.dye.info[1], dye_wh);

  // velocity, double buffer
  create_tex(data.velocity.info[0], sim_wh);
  create_tex(data.velocity.info[1], sim_wh);

  // divergence
  create_tex(data.divergence.info, sim_wh);

  // pressure, double buffer
  create_tex(data.pressure.info[0], sim_wh);
  create_tex(data.pressure.info[1], sim_wh);
};

void
setup_fluidsim_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = get_pass_idx(ri, PassName::fluid_sim);
  auto& pass = ri.passes[pass_idx];
  const int tex_unit = get_tex_unit(ri, PassName::fluid_sim);

  pass.update = [&pass, tex_unit](entt::registry& r, const float dt) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    auto& data = ri.fluid_sim;

    const auto wh = ri.viewport_size_render_at;

    // do all the fluid sim steps...

    data.advectProgram.bind();

    // self advection
    {
      const auto w_vel = data.velocity.info[data.velocity.index];
      const auto r_vel = data.velocity.info[1 - data.velocity.index];
      const auto texel_size = glm::vec2{ 1.0 / r_vel.tex.size.x, 1.0f / r_vel.tex.size.y };

      data.advectProgram.set_vec2("texel_size", texel_size);
      data.advectProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.advectProgram.set_int("u_source", r_vel.tex.tex_unit.unit);
      data.advectProgram.set_float("dt", dt);
      data.advectProgram.set_float("dissapation", data.config_velocity_dissapation);

      const auto fbo = w_vel.fbo_id;
      engine::Framebuffer::bind_fbo(fbo);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0 });
      engine::RenderCommand::clear();

      // render something to the fbo...
      render_fullscreen_quad(r, data.advectProgram, wh);

      // swap the buffer.
      data.velocity.swap();
    }

    // dye advection
    {
      const auto r_vel = data.velocity.info[1 - data.velocity.index]; // get the vel just updated
      const auto r_dye = data.dye.info[1 - data.dye.index];
      const auto w_dye = data.dye.info[data.dye.index];
      const auto texel_size = glm::vec2{ 1.0 / r_vel.tex.size.x, 1.0f / r_vel.tex.size.y };

      data.advectProgram.set_vec2("texel_size", texel_size);
      data.advectProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.advectProgram.set_int("u_source", r_dye.tex.tex_unit.unit);
      data.advectProgram.set_float("dissapation", data.config_density_dissapation);

      // render something to the fbo...
      render_fullscreen_quad(r, data.advectProgram, wh);

      // swap the buffer
      data.dye.swap();
    }

    // Finlly, render the the out in to the assigned renderpass texture...
    // output final fluidsim texture to renderpass tetxure
    engine::Framebuffer::bind_fbo(pass.fbos[0]);
    engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
    engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0 });
    engine::RenderCommand::clear();
    render_fullscreen_quad(r, data.advectProgram, wh);
  };
};

} // namespace game2d