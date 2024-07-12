#include "components.hpp"

#include "magic_enum.hpp"

#include <fmt/core.h>

namespace game2d {

static int global_tex_unit_count = 0;

RenderPass::RenderPass(const PassName& pass_name, const int colour_buffers)
  : pass(pass_name)
  , colour_buffers_per_texture(colour_buffers){};

void
RenderPass::setup(const glm::ivec2& fbo_size, const int framebuffers)
{
  // Create all the textures
  for (int i = 0; i < framebuffers; i++) {
    for (int j = 0; j < colour_buffers_per_texture; j++) {
      Texture t;
      t.tex_unit = TextureUnit(global_tex_unit_count++);
      t.tex_id = TextureId{ 0 };
      t.path = std::string(magic_enum::enum_name(pass)) + std::to_string(i);
      texs.push_back(t);
    }
  }

  int tex_idx = 0;

  for (int n_fbos = 0; n_fbos < framebuffers; n_fbos++) {

    std::optional<engine::FboResult> result_opt = std::nullopt;
    if (framebuffers == 1) {
      // use the first texture unit
      // later, we use glActiveTextre & glBindTexture to link
      // the extra crated texture unit to the additional colour buffer.
      result_opt = engine::new_texture_to_fbo(texs[0].tex_unit.unit, fbo_size, colour_buffers_per_texture);
    } else
      result_opt = engine::new_texture_to_fbo(texs[tex_idx].tex_unit.unit, fbo_size, colour_buffers_per_texture);

    const auto result = result_opt.value();
    fbos.push_back(result.out_fbo_id);

    for (int i = 0; i < result.out_tex_ids.size(); i++) {
      texs[tex_idx].tex_id.id = result.out_tex_ids[i];
      tex_idx++;
    }
  }
};

} // namespace game2d