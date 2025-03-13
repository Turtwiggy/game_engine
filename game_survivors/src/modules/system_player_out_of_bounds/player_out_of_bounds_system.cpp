#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"
#include "player_out_of_bounds_system.hpp"

namespace game2d {

void
update_player_out_of_bounds_system(entt::registry& r)
{
  constexpr int map_radius = 800;
  constexpr int map_radius_sqr = map_radius * map_radius;

  for (const auto& [e, player_c] : r.view<PlayerComponent>().each()) {

    const auto d = get_position(r, e) - glm::vec2{ 0, 0 };
    const auto d2 = d.x * d.x + d.y * d.y;
    ImGui::Text("d2: %f d2_max: %i", d2, map_radius_sqr);

    if (d2 > map_radius) {
      // start to apply damage to player!;

      Sprite s;
      s.pos = d;
      s.sprite = "CROSSHAIR_1";
      s.size = { 16, 16 };
      draw_sprite(r, s);
    }

    //
  }
}

} // namespace game2d