#include "pch.hpp"

#include "components.hpp"

#include "engine/opengl/texture.hpp"

namespace game2d {

static int global_tex_unit_count = 0;

SINGLE_RendererInfo SINGLE_RendererInfo::instance;

// RenderPass::RenderPass(const PassName& pass_name, const int colour_buffers)
//   : pass(pass_name)
//   , colour_buffers_per_texture(colour_buffers) {};

void
setup_rp(RenderPass& rp, const glm::ivec2& fbo_size, const int framebuffers)
{
  // Setup the texture objects
  for (int i = 0; i < framebuffers; i++) {
    for (int j = 0; j < rp.colour_buffers_per_texture; j++) {
      engine::Texture t;
      t.tex_unit = engine::TextureUnit(global_tex_unit_count++);
      t.tex_id = engine::TextureId{ 0 };
      t.path = std::string(magic_enum::enum_name(rp.pass)) + std::to_string(i);
      rp.texs.push_back(t);
    }
  }

  int tex_idx = 0;

  engine::TextureFiltering f;
  f.texture_wrap_s = engine::Filtering::clamp_to_border;
  f.texture_wrap_t = engine::Filtering::clamp_to_border;
  f.texture_min_filter = engine::Filtering::linear;
  f.texture_mag_filter = engine::Filtering::linear;
  // SDL_Log("Creating fbo texture for rp: %s", std::string(magic_enum::enum_name(rp.pass)).c_str());

  for (int n_fbos = 0; n_fbos < framebuffers; n_fbos++) {

    std::optional<engine::FboResult> result_opt = std::nullopt;
    if (framebuffers == 1) {
      // use the first texture unit
      // later, we use glActiveTextre & glBindTexture to link
      // the extra crated texture unit to the additional colour buffer.
      result_opt = engine::new_texture_to_fbo(rp.texs[0].tex_unit.unit, fbo_size, f, rp.colour_buffers_per_texture);
    } else
      result_opt = engine::new_texture_to_fbo(rp.texs[tex_idx].tex_unit.unit, fbo_size, f, rp.colour_buffers_per_texture);

    const auto result = result_opt.value();
    rp.fbos.push_back(result.out_fbo_id);

    for (size_t i = 0; i < result.out_tex_ids.size(); i++) {
      rp.texs[tex_idx].tex_id.id = result.out_tex_ids[i];
      tex_idx++;
    }
  }
};

} // namespace game2d