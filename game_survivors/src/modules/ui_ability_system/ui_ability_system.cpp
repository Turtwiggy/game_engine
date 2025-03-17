#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"
#include "modules/system_ability/ability_components.hpp"
#include "ui_ability_system.hpp"

namespace game2d {

void
update_ui_ability_system(entt::registry& r)
{

  for (const auto& [e, ability_c, colour_c, t_c] :
       r.view<const AbilityComponent, const DefaultColour, const TransformComponent>().each()) {
    const bool allowed_to_use_ability_1 = ability_c.ability_1_cooldown_left <= 0.0f;
    const bool allowed_to_use_ability_2 = ability_c.ability_2_cooldown_left <= 0.0f;

    const float icon_padding = 6.0f;
    const auto ability_1_icon_size = glm::vec2{ 4, 4 };
    const auto ability_2_icon_size = glm::vec2{ 4, 4 };
    const auto ability_1_spot = glm::vec2{ t_c.scale.x, t_c.scale.y };
    const auto ability_2_spot = glm::vec2{ ability_1_spot.x + ability_1_icon_size.x + icon_padding, ability_1_spot.y };
    const auto cooldown_col = engine::SRGBColour(0.2f, 0.2f, 0.2f, 0.75f);
    const auto active_col = engine::SRGBColour(0.2f, 1.0f, 0.2f, 0.75f);

    Sprite s1;
    s1.sprite = allowed_to_use_ability_1 ? "CIRCLE" : "CIRCLE";
    s1.col = allowed_to_use_ability_1 ? active_col : cooldown_col;
    s1.pos = get_position(r, e) + ability_1_spot;
    s1.size = { 8, 8 };
    s1.z_idx = ZLayer::VFX;
    draw_sprite(r, s1);

    Sprite s2;
    s2.sprite = allowed_to_use_ability_2 ? "CIRCLE" : "CIRCLE";
    s2.col = allowed_to_use_ability_2 ? active_col : cooldown_col;
    s2.pos = get_position(r, e) + ability_2_spot;
    s2.size = { 8, 8 };
    s2.z_idx = ZLayer::VFX;
    draw_sprite(r, s2);

    // draw an anchor sprite
    if (ability_c.ability_1_in_progress) {
      const auto offset = glm::vec2{ -10, 10 };
      Sprite s;
      s.sprite = "PICKAXE"; // kinda looks like anchor if you rotate it
      // point pickaxe down and left
      constexpr float rotation = engine::PI + (30 * engine::Deg2Rad);
      // imgui_draw_float("rotation", rotation);
      s.z_rotation = rotation;
      s.col = colour_c.colour;
      s.pos = get_position(r, e) + offset;
      s.size = { 20, 20 };
      s.z_idx = ZLayer::VFX;
      draw_sprite(r, s);
    }
  }
}

} // namespace game2d