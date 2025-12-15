#include "pch.hpp"

#include "sprite_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/renderer/helpers.hpp"

namespace game2d {

void
begin_frame_sprite(entt::registry& r)
{
  auto& imsprite = get_first_component<SINGLE_ImSprite>(r);
  auto& pool = imsprite.pool;
  auto& cached = imsprite.cached;
  auto& used = imsprite.used;

  Sprite defaults;
  for (int i = 0; i < used; i++) {
    auto e = pool.instances[i];
    set_sprite(r, e, defaults.sprite);
    set_position(r, e, defaults.pos);
    set_size(r, e, defaults.size);
    set_z_index(r, e, defaults.z_idx);
    set_colour(r, e, defaults.col);
  }
  ImGui::Text("Used: %i", used);

  pool.update(r, cached);
  used = 0;
};

void
draw_sprite(entt::registry& r, const Sprite& desc)
{
  auto& imsprite = get_first_component<SINGLE_ImSprite>(r);
  auto& pool = imsprite.pool;
  auto& cached = imsprite.cached;
  auto& used = imsprite.used;

  auto e = pool.instances[used++];
  set_sprite(r, e, desc.sprite);
  set_position(r, e, desc.pos);
  set_size(r, e, desc.size);
  set_z_index(r, e, desc.z_idx);
  set_colour(r, e, desc.col);

  // double the cache size if we ever go over it
  if (used >= cached) {
    cached *= 2;
    pool.update(r, cached);
    SDL_Log("pool count hit... doubling cached size");
  };
};

} // namespace game2d