#include "combat_scale_on_hit_system.hpp"

#include "combat_scale_on_hit_components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat/combat_scale_on_hit/helpers.hpp"
#include "modules/core/colour/components.hpp"

namespace game2d {

void
enable_flash(entt::registry& r, entt::entity e)
{
  // set_colour(r, e, get_srgb_colour_by_tag(r, "enemy_flash"));
  set_colour(r, e, { 1.0f, 1.0f, 1.0f, 1.0f });
};

void
disable_flash(entt::registry& r, entt::entity e)
{
  const auto& default_c = r.get<DefaultColour>(e);
  set_colour(r, e, default_c.colour);
};

// note: while the thing is scaling, also make it flash
void
update_combat_scale_on_hit_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& view = r.view<RequestHitScaleComponent>(entt::exclude<WaitForInitComponent>);

  // note: req_e is attached to the parent
  for (const auto& [e, req_c] : view.each()) {

    std::vector<entt::entity> entities_to_flash;
    if (r.all_of<SpriteComponent>(e))
      entities_to_flash.push_back(e);
    if (r.all_of<HasChildrenComponent>(e)) {
      const auto& children_c = r.get<HasChildrenComponent>(e);
      for (const auto child_e : children_c.children) {
        if (!r.all_of<DefaultSizeComponent>(child_e)) {
          SDL_Log("Warning: sprite to scale doesnt have defaultsizecomponent");
          continue;
        }
        if (r.all_of<WeaponComponent>(child_e))
          continue;
        if (r.all_of<SpriteComponent>(child_e))
          entities_to_flash.push_back(child_e);
      }
    }

    if (req_c.t == 0.0f)
      std::ranges::for_each(entities_to_flash, [&r](auto e) { enable_flash(r, e); });

    // variables
    const float speed = 5.0f;
    const float scale_up = 5.0f;
    req_c.t += dt * speed;

    const float scale = spike(req_c.t);

    for (const auto e_to_flash : entities_to_flash) {
      const auto& default_size_c = r.get<DefaultSizeComponent>(e_to_flash);
      auto new_x = default_size_c.size.x + scale_up * scale;
      auto new_y = default_size_c.size.y + scale_up * scale;
      set_size(r, e_to_flash, { new_x, new_y });
    }

    if (req_c.t >= 1.0f) {
      std::ranges::for_each(entities_to_flash, [&r](auto e) { disable_flash(r, e); });

      for (const auto e_to_flash : entities_to_flash) {
        const auto& default_size_c = r.get<DefaultSizeComponent>(e_to_flash);
        set_size(r, e_to_flash, default_size_c.size);
      }

      r.remove<RequestHitScaleComponent>(e);
    }
  }

  //
}

} // namespace game2d