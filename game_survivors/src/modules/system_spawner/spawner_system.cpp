#include "spawner_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/effects_outline/outline_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_items_drop_on_death/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "spawner_components.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

glm::ivec2
rnd_position_off_screen(entt::registry& r, const glm::ivec2 center)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  static engine::RandomState rnd(0);
  const float rnd_val_0 = engine::rand_01(rnd.rng);

  // generate a random angle 0 to 2PI
  float angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);

  // generate a random distance outside the radius
  // float radius = std::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  float distance = 200;

  const auto dir = engine::angle_radians_to_direction(angle);
  float spawn_x = center.x + dir.x * distance;
  float spawn_y = center.y + dir.y * distance;
  return { spawn_x, spawn_y };
};

void
update_spawner_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  // should vary targets, not just be first player
  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  const auto player_t = r.get<TransformComponent>(player_e);

  for (const auto& [spawner_e, spawner_c, cooldown_c] : r.view<SpawnerComponent, CooldownComponent>().each()) {
    //
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    std::string enemy_key = "actor_enemy_melee";

    auto e = spawn(r, enemy_key);
    r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::enemy });
    r.emplace<DynamicTargetComponent>(e, player_e);
    // r.emplace<SpriteOutline>(e);

    // temp: health here, but it should be based on wave/enemytype
    r.emplace_or_replace<HealthComponent>(e, 30, 30);

    // move at player, this gotta be changed for more interesting types
    r.emplace<PhysicsDynamicTarget>(e, player_e);
    ApplyForceToDynamicTarget tgt_c;
    tgt_c.orbit = true;
    tgt_c.reduce_thrusters = false;
    tgt_c.speed = 10.0f;
    r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);

    OnDeathCallback callback;
    callback.callback = [](entt::registry& r, const entt::entity e) {
      SDL_Log("Calling drop_xp_on_death_callback()");
      drop_xp_on_death_callback(r, e);
    };
    r.emplace<OnDeathCallback>(e, callback);

    // get a random position off screen
    const auto rnd_pos = rnd_position_off_screen(r, { player_t.position.x, player_t.position.y });
    give_life(r, e, rnd_pos, { 16, 16 });
  }
}

} // namespace game2d