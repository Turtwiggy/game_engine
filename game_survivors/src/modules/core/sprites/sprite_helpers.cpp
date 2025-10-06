#include "pch.hpp"

#include "sprite_helpers.hpp"

#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"

namespace game2d {

void
begin_frame_sprite(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& imsprite = get_first_component<SINGLE_ImSprite>(r);
  auto& pool = imsprite.pool;
  auto& cached = imsprite.cached;

  pool.update(r, cached);
  imsprite.sprites.clear();
};

void
draw_sprite(entt::registry& r, const Sprite& desc)
{
  auto& imsprite = get_first_component<SINGLE_ImSprite>(r);
  auto& pool = imsprite.pool;
  auto& cached = imsprite.cached;
  auto used = (int)imsprite.sprites.size();

  // double the cache size if we ever go over it
  if (used >= cached) {
    imsprite.cached *= 2;
    pool.update(r, imsprite.cached);
    SDL_Log("pool count hit... doubling cached size (new: %i)", imsprite.cached);
  };

  imsprite.sprites.push_back(desc);
};

void
draw_all_sprites(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& imsprite = get_first_component<SINGLE_ImSprite>(r);
  auto& sprites = imsprite.sprites;

  int i = 0;
  int used = imsprite.sprites.size();

  const auto view = r.view<const EntityPoolComponent, SpriteComponent, TransformComponent>();
  for (const auto& [e, pool_c, spr_c, t_c] : view.each()) {

    if (i < used) {

      const auto& spr = sprites[i];
      set_sprite(r, e, spr.sprite);
      t_c.position = { spr.pos.x, spr.pos.y, 0.0f };
      t_c.scale = { spr.size.x, spr.size.y, 1.0f };
      t_c.z_index = (int)spr.z_idx;
      t_c.rotation_radians.z = spr.z_rotation;
      spr_c.colour = engine::SRGBToLinear(spr.col);

    } else {
      // hide the unused sprite.
      t_c.scale = { 0.0f, 0.0f, 0.0f };
    }

    i++;
  }
}

} // namespace game2d