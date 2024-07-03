#include "components.hpp"

#include "magic_enum.hpp"

namespace game2d {

static int global_tex_unit_count = 0;

RenderPass::RenderPass(const PassName& pass_name, const int colour_attachments)
  : pass(pass_name)
  , colour_buffers(colour_attachments)
{
  for (int i = 0; i < colour_attachments; i++) {
    Texture t;
    t.tex_unit = TextureUnit(global_tex_unit_count++);
    t.tex_id = TextureId(0);
    t.path = std::string(magic_enum::enum_name(pass_name)) + std::to_string(i);
    texs.push_back(t);
  }
};

void
RenderPass::setup(const glm::ivec2& fbo_size)
{
  // use the first texture unit
  // later, we use glActiveTextre & glBindTexture to link
  // the extra crated texture unit to the additional colour buffer.
  const auto result = engine::new_texture_to_fbo(texs[0].tex_unit.unit, fbo_size, colour_buffers);
  fbo = result.out_fbo_id;

  for (int i = 0; i < result.out_tex_ids.size(); i++)
    texs[i].tex_id.id = result.out_tex_ids[i];
};

} // namespace game2d