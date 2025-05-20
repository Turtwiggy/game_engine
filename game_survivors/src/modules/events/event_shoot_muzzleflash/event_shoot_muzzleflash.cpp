#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/systems/system_particles/components.hpp"
#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "event_shoot_muzzleflash.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_alpha_based_on_lifecycle/alpha_based_on_lifecycle_components.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"
#include "modules/ui/ui_debug_effects/effects_helpers.hpp"

namespace game2d {

static float s = 24.0f;
static float scalar = 0.2f;

void
update_muzzleflash_system(entt::registry& r)
{
  // imgui_draw_float("s", s);
  // imgui_draw_float("scalar", scalar);
}

void
handle_shoot_event__muzzleflash(entt::registry& r, const ShootEvent& evt)
{
  const auto par_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (!r.all_of<PlayerComponent>(par_e))
    return;

  // spawn a muzzle vfx
  const auto rot = r.get<TransformComponent>(wep_e).rotation_radians.z;
  const auto size = glm::vec2{ s, s };
  const auto off = engine::angle_radians_to_direction(rot) * scalar * size;
  const auto pos = get_position(r, wep_e) + off;

  const auto e = spawn_fx(r, "S6_EXPLODE_FX_21", pos, size, 30.0f);
  r.emplace<SetPositionAtDynamicTarget>(e, off);
  r.emplace<DynamicTargetComponent>(e, wep_e);

  // note: adjust for half pi, because the spritesheet effect is pointing upwards,
  // but we want it to be pointing right to match the "forward" direction
  r.get<TransformComponent>(e).rotation_radians.z = rot + engine::HALF_PI;

  set_z_index(r, e, ZLayer::VFX);
}

} // namespace game2d