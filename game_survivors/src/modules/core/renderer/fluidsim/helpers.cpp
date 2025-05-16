#include "pch.hpp"

#include "modules/core/renderer/fluidsim/helpers.hpp"

#include "engine/deps/opengl.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/fluidsim/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"

namespace game2d {
using namespace engine; // for macro

#define FLUIDSIM_DISABLED 1

const auto render_quad = [](entt::registry& r, const engine::Shader& shader, const glm::ivec2& size) {
  auto& ri = SINGLE_RendererInfo::instance;

  ri.renderer.reset_quad_vert_count();
  ri.renderer.begin_batch();

  engine::quad_renderer::RenderDescriptor desc;
  // desc.pos_tl = { -size.x * 0.5f, -size.y * 0.5f };
  desc.pos_tl = { 0, 0 };
  desc.size = size;
  desc.yaw_pitch_roll_radians = { 0, 0, 0 };

  ri.renderer.draw_sprite(desc, shader);

  ri.renderer.end_batch();
  ri.renderer.flush(shader);
};

void
rebind_fluidsim(entt::registry& r, FluidSimData& data)
{
#if defined(FLUIDSIM_DISABLED)
  return;
#endif

  // velocity, double buffer
  glActiveTexture(GL_TEXTURE0 + data.velocity.info[0].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.velocity.info[0].tex.tex_id.id);
  glActiveTexture(GL_TEXTURE0 + data.velocity.info[1].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.velocity.info[1].tex.tex_id.id);

  // dye, double buffer
  glActiveTexture(GL_TEXTURE0 + data.dye.info[0].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.dye.info[0].tex.tex_id.id);
  glActiveTexture(GL_TEXTURE0 + data.dye.info[1].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.dye.info[1].tex.tex_id.id);

  // divergence
  glActiveTexture(GL_TEXTURE0 + data.divergence.info.tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.divergence.info.tex.tex_id.id);

  // curl
  glActiveTexture(GL_TEXTURE0 + data.curl.info.tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.curl.info.tex.tex_id.id);

  // pressure, double buffer
  glActiveTexture(GL_TEXTURE0 + data.pressure.info[0].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.pressure.info[0].tex.tex_id.id);
  glActiveTexture(GL_TEXTURE0 + data.pressure.info[1].tex.tex_unit.unit);
  glBindTexture(GL_TEXTURE_2D, data.pressure.info[1].tex.tex_id.id);
};

void
load_fluidsim(entt::registry& r, FluidSimData& data, int& used_texture_units)
{
#if defined(FLUIDSIM_DISABLED)
  return;
#endif

  data.splatProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/splat.frag");
  data.advectProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/advect.frag");
  data.curlProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/curl.frag");
  data.vorticityProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/vorticity.frag");
  data.divergenceProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/divergence.frag");
  data.pressureProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/pressure.frag");
  data.gradientSubtractProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/gradient_sub.frag");
  data.textureProgram = engine::Shader(r, "assets/shaders/sim/fluid.vert", "assets/shaders/sim/texture.frag");

  const auto sim_wh = glm::ivec2{ data.config_sim_resolution, data.config_sim_resolution };
  const auto dye_wh = glm::ivec2{ data.config_dye_resolution, data.config_dye_resolution };

  const auto create_tex = [&used_texture_units](FboAndTexInfo& out, glm::ivec2 wh, int filtering) {
    DoubleBufferInfo info;

    // assign a texture unit
    Texture tex;
    tex.tex_unit.unit = used_texture_units;
    tex.size = wh;

    engine::TextureFiltering f;
    f.texture_wrap_s = GL_CLAMP_TO_EDGE;
    f.texture_wrap_t = GL_CLAMP_TO_EDGE;
    f.texture_min_filter = filtering;
    f.texture_mag_filter = filtering;

    // generate a texture and attach to fbo
    const engine::FboResult res = engine::new_texture_to_fbo(tex.tex_unit.unit, wh, f);
    tex.tex_id.id = res.out_tex_ids[0];

    out.tex = tex;
    out.fbo_id = res.out_fbo_id;
    used_texture_units++; // increment after
  };

  create_tex(data.dye.info[0], dye_wh, GL_LINEAR);
  create_tex(data.dye.info[1], dye_wh, GL_LINEAR);
  create_tex(data.velocity.info[0], sim_wh, GL_LINEAR);
  create_tex(data.velocity.info[1], sim_wh, GL_LINEAR);
  create_tex(data.divergence.info, sim_wh, GL_NEAREST);
  create_tex(data.curl.info, sim_wh, GL_NEAREST);
  create_tex(data.pressure.info[0], sim_wh, GL_NEAREST);
  create_tex(data.pressure.info[1], sim_wh, GL_NEAREST);

  CHECK_OPENGL_ERROR(321123);
};

int
get_texs_used_by_fluidsim()
{
#if defined(FLUIDSIM_DISABLED)
  return 0;
#endif
  return 8;
}

void
setup_fluidsim_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::fluid_sim);
  if (pass_idx == -1)
    return;
  auto& pass = ri.passes[pass_idx];
  const int tex_unit = get_tex_unit(ri, PassName::fluid_sim);

  pass.update = [&pass](entt::registry& r, const float dt, const glm::vec2& mouse_pos) {
    auto& ri = SINGLE_RendererInfo::instance;
    auto& input_c = get_first_component<SINGLE_InputComponent>(r);
    auto& data = ri.fluid_sim;

// fluidsim disabled
// note for future: something is wrong with the mouse_pos to simspace conversion
#if defined(FLUIDSIM_DISABLED)
    return;
#endif

    // mouse info
    static glm::vec2 prev_frame_pos{ 0, 0 };
    const auto m0_held = ImGui::IsMouseDown(0);
    const auto m1_held = ImGui::IsMouseDown(1);

    // convert raw mouse position relative to camera,
    // and relative to the top left of the fluidsim
    // note: 4096 is the size of the transform
    const glm::vec2 fluidsim_tl = { -4096 * 0.5f, -4096 * 0.5f };
    const glm::vec2 simspace = { mouse_pos.x - fluidsim_tl.x, mouse_pos.y - fluidsim_tl.y };
    const float point_in_0_to_1_x = glm::clamp(simspace.x / data.config_dye_resolution, 0.0f, 1.0f);
    const float point_in_0_to_1_y = glm::clamp(simspace.y / data.config_dye_resolution, 0.0f, 1.0f);
    glm::vec2 point = { point_in_0_to_1_x, point_in_0_to_1_y };

    ImVec2 dxdy = { 0, 0 };
    if (ImGui::IsMouseDragging(1)) {
      ImVec2 dir = { point.x - prev_frame_pos.x, point.y - prev_frame_pos.y };
      prev_frame_pos = point;
      const auto nrm = engine::normalize_safe({ dir.x, dir.y });
      dxdy.x = nrm.x * data.config_splat_force;
      dxdy.y = nrm.y * data.config_splat_force;
    }

    ImGui::Begin("DebugFluid");
    glm::vec2 glm_mpos = { simspace.x, simspace.y };
    glm::vec2 glm_dxdy = { dxdy.x, dxdy.y };
    imgui_draw_vec2("mouse_pos", glm_mpos);
    imgui_draw_vec2("dxdy", glm_dxdy);
    imgui_draw_vec2("point", point);
    imgui_draw_float("config_density_dissapation", data.config_density_dissapation);
    imgui_draw_float("config_velocity_dissapation", data.config_velocity_dissapation);
    imgui_draw_float("config_pressure", data.config_pressure);
    imgui_draw_float("config_pressure_iterations", data.config_pressure_iterations);
    imgui_draw_float("config_splat_radius", data.config_splat_radius);
    imgui_draw_float("config_splat_force", data.config_splat_force);
    imgui_draw_float("config_curl", data.config_curl);
    imgui_draw_bool("config_shading", data.config_shading);

    static float col[3] = { 0.1f, 0.5f, 0.9f };
    col[0] = data.config_dye_colour.r;
    col[1] = data.config_dye_colour.g;
    col[2] = data.config_dye_colour.b;
    ImGui::ColorEdit3("dye col", col);
    data.config_dye_colour.r = col[0];
    data.config_dye_colour.g = col[1];
    data.config_dye_colour.b = col[2];

    ri.instanced.bind();
    ri.instanced.set_bool("tex_fluid_shading", data.config_shading);

    // run sim at 60fps
    static float dt_max = 1 / 60.0f;
    static float dt_cur = 0.0f;
    dt_cur += dt;
    if (dt_cur < dt_max) {
      ImGui::End();
      return;
    }
    dt_cur -= dt_max;

    // start fluid sim...
    const auto sim_texel_size = glm::vec2{ 1.0 / data.config_sim_resolution, 1.0 / data.config_sim_resolution };
    const auto dye_texel_size = glm::vec2{ 1.0 / data.config_dye_resolution, 1.0 / data.config_dye_resolution };

    // self advection
    {
      const auto r_vel = data.velocity.read();
      const auto w_vel = data.velocity.write();
      const auto texel_size = glm::vec2{ 1.0f / w_vel.tex.size.x, 1.0f / w_vel.tex.size.y };

      data.advectProgram.bind();
      data.advectProgram.set_float("dt", dt_max);
      data.advectProgram.set_vec2("texel_size", texel_size);
      data.advectProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.advectProgram.set_int("u_source", r_vel.tex.tex_unit.unit);
      data.advectProgram.set_float("dissapation", data.config_velocity_dissapation);

      const auto wh = glm::ivec2{ w_vel.tex.size.x, w_vel.tex.size.y };
      engine::Framebuffer::bind_fbo(w_vel.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
      engine::RenderCommand::clear();

      render_quad(r, data.advectProgram, wh);

      data.velocity.swap(); // swap the buffer.
    }

    // dye advection
    {
      const auto r_vel = data.velocity.read();
      const auto r_dye = data.dye.read();
      const auto w_dye = data.dye.write();
      const auto texel_size = glm::vec2{ 1.0f / r_vel.tex.size.x, 1.0f / r_vel.tex.size.y };
      // const auto dye_texel_size = glm::vec2{ 1.0f / w_dye.tex.size.x, 1.0f / w_dye.tex.size.y };

      data.advectProgram.bind();
      data.advectProgram.set_float("dt", dt_max);
      data.advectProgram.set_vec2("texel_size", texel_size);
      data.advectProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.advectProgram.set_int("u_source", r_dye.tex.tex_unit.unit);
      data.advectProgram.set_float("dissapation", data.config_density_dissapation);

      const auto wh = glm::ivec2{ w_dye.tex.size.x, w_dye.tex.size.y };
      engine::Framebuffer::bind_fbo(w_dye.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
      engine::RenderCommand::clear();

      render_quad(r, data.advectProgram, wh);

      data.dye.swap();
    }

    // external forces
    if (m1_held) {
      // splats update velocity texture
      {
        const auto r_vel = data.velocity.read();
        const auto w_vel = data.velocity.write();
        const auto wh = glm::ivec2{ w_vel.tex.size.x, w_vel.tex.size.y };

        data.splatProgram.bind();
        data.splatProgram.set_vec2("point", point);
        data.splatProgram.set_float("radius", data.config_splat_radius / 100.0f);
        data.splatProgram.set_int("u_target", r_vel.tex.tex_unit.unit);
        data.splatProgram.set_vec3("colour", { dxdy.x, dxdy.y, 0.0f });

        engine::Framebuffer::bind_fbo(w_vel.fbo_id);
        engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
        engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
        engine::RenderCommand::clear();
        render_quad(r, data.splatProgram, wh);

        data.velocity.swap();
      }
    }
    if (m0_held) {
      // splats update dye texture
      {
        const auto r_dye = data.dye.read();
        const auto w_dye = data.dye.write();
        const auto wh = glm::ivec2{ w_dye.tex.size.x, w_dye.tex.size.y };

        data.splatProgram.bind();
        data.splatProgram.set_vec2("point", point);
        data.splatProgram.set_float("radius", data.config_splat_radius / 100.0f);
        data.splatProgram.set_int("u_target", r_dye.tex.tex_unit.unit);
        data.splatProgram.set_vec3("colour", data.config_dye_colour);

        engine::Framebuffer::bind_fbo(w_dye.fbo_id);
        engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
        engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
        engine::RenderCommand::clear();
        render_quad(r, data.splatProgram, wh);

        data.dye.swap();
      }
    }

    /*
    // curl
    {
      const auto w_curl = data.curl.info;
      const auto r_vel = data.velocity.read();
      const auto wh = glm::ivec2{ r_vel.tex.size.x, r_vel.tex.size.y };
      const auto texel_size = sim_texel_size;
      data.curlProgram.bind();
      data.curlProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.curlProgram.set_vec2("texel_size", texel_size);
      engine::Framebuffer::bind_fbo(w_curl.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
      engine::RenderCommand::clear();
      render_quad(r, data.curlProgram, wh);
    }

    // vorticity
    {
      const auto r_vel = data.velocity.read();
      const auto r_curl = data.curl.info;
      const auto w_vel = data.velocity.write();
      const auto wh = glm::ivec2{ w_vel.tex.size.x, w_vel.tex.size.y };
      data.vorticityProgram.bind();
      data.vorticityProgram.set_vec2("texel_size", sim_texel_size);
      data.vorticityProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.vorticityProgram.set_int("u_curl", r_curl.tex.tex_unit.unit);
      data.vorticityProgram.set_float("dt", dt_max);
      data.vorticityProgram.set_float("curl", data.config_curl);
      engine::Framebuffer::bind_fbo(w_vel.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      render_quad(r, data.vorticityProgram, wh);
      data.velocity.swap();
    }
    */

    // divergence
    {
      const auto r_vel = data.velocity.read();
      const auto w_div = data.divergence.info;
      const auto texel_size = glm::vec2{ 1.0f / w_div.tex.size.x, 1.0f / w_div.tex.size.y };

      const auto wh = glm::ivec2{ w_div.tex.size.x, w_div.tex.size.y };
      engine::Framebuffer::bind_fbo(w_div.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
      engine::RenderCommand::clear();

      data.divergenceProgram.bind();
      data.divergenceProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);
      data.divergenceProgram.set_vec2("texel_size", texel_size);

      render_quad(r, data.divergenceProgram, wh);
    }

    // clear pressure to a value.
    {
      const auto r_pressure = data.pressure.read();
      const auto w_pressure = data.pressure.write();

      data.textureProgram.bind();
      data.textureProgram.set_float("val", data.config_pressure);
      data.textureProgram.set_int("tex", r_pressure.tex.tex_unit.unit);

      const auto wh = glm::ivec2{ w_pressure.tex.size.x, w_pressure.tex.size.y };
      engine::Framebuffer::bind_fbo(w_pressure.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
      engine::RenderCommand::clear();

      render_quad(r, data.textureProgram, wh);

      data.pressure.swap();
    }

    // pressure iterations
    {
      const auto r_divergence = data.divergence.info;
      const auto texel_size = sim_texel_size;

      for (int i = 0; i < data.config_pressure_iterations; i++) {

        const auto r_pressure = data.pressure.read();
        const auto w_pressure = data.pressure.write();

        data.pressureProgram.bind();
        data.pressureProgram.set_int("u_divergence", r_divergence.tex.tex_unit.unit);
        data.pressureProgram.set_vec2("texel_size", texel_size);
        data.pressureProgram.set_int("u_pressure", r_pressure.tex.tex_unit.unit);

        const auto wh = glm::ivec2{ w_pressure.tex.size.x, w_pressure.tex.size.y };
        engine::Framebuffer::bind_fbo(w_pressure.fbo_id);
        engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
        engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
        engine::RenderCommand::clear();

        render_quad(r, data.pressureProgram, wh);

        data.pressure.swap();
      }
    }

    // gradient subtract
    {
      const auto r_vel = data.velocity.read();
      const auto r_pressure = data.pressure.read();
      const auto w_vel = data.velocity.write();
      const auto texel_size = sim_texel_size;

      data.gradientSubtractProgram.bind();
      data.gradientSubtractProgram.set_vec2("texel_size", texel_size);
      data.gradientSubtractProgram.set_int("u_pressure", r_pressure.tex.tex_unit.unit);
      data.gradientSubtractProgram.set_int("u_velocity", r_vel.tex.tex_unit.unit);

      const auto wh = glm::ivec2{ w_vel.tex.size.x, w_vel.tex.size.y };
      engine::Framebuffer::bind_fbo(w_vel.fbo_id);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
      engine::RenderCommand::clear();

      render_quad(r, data.gradientSubtractProgram, wh);

      data.velocity.swap();
    }

    //
    // Finlly, render the results (of the dye?) in to the assigned renderpass texture...
    //

    data.textureProgram.bind();
    data.textureProgram.set_float("val", 1.0f);
    data.textureProgram.set_int("tex", data.dye.read().tex.tex_unit.unit);

    const auto res = data.config_dye_resolution;

    engine::Framebuffer::bind_fbo(pass.fbos[0]);
    engine::RenderCommand::set_viewport(0, 0, res, res);
    engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 255 });
    engine::RenderCommand::clear();
    render_quad(r, data.textureProgram, { res, res });

    ImGui::End();
  };
};

} // namespace game2d