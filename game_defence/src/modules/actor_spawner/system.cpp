#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

// each wave should have a "theme"
// e.g. flying, horse-back, etc
// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};

void
update_spawner_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  // const float dt = milliseconds_dt / 1000.0f;
  static engine::RandomState rnd;

  const auto& player_view = r.view<const PlayerComponent, const AABB>();

  const auto& view = r.view<SpawnerComponent, AttackCooldownComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, spawner, cooldown] : view.each()) {

    if (cooldown.on_cooldown)
      continue;

    // Check if any players are in range
    bool ok_to_spawn = true;
    if (const auto* player_constraint = r.try_get<OnlySpawnInRangeOfAnyPlayerComponent>(entity)) {
      // Assume this spawner has a physical position
      const auto* transform = r.try_get<TransformComponent>(entity);
      if (transform != nullptr) {
        for (const auto& [player_e, player_comp, player_aabb] : player_view.each()) {
          const glm::vec2 d = glm::vec2(player_aabb.center) - glm::vec2(transform->position.x, transform->position.y);
          const float current_distance2 = d.x * d.x + d.y * d.y;
          if (current_distance2 > player_constraint->distance2) {
            ok_to_spawn = false;
            break;
          }
        }
      }
    }
    if (!ok_to_spawn)
      continue; // dont activate this spawner. No players in range.

    if (spawner.continuous_spawn) {

      // TODO: replace to spawn with multiple types
      const auto& first_type = spawner.types_to_spawn[0];

      const auto e = create_gameplay(r, first_type);

      // Set spawn position as physical position
      const auto* transform = r.try_get<TransformComponent>(entity);
      if (transform != nullptr)
        set_position(r, e, { transform->position.x, transform->position.y });

      // Set position based on a boundingbox
      // generate a random position in the boundingbox
      if (spawner.spawn_in_boundingbox) {
        const auto& aabb = spawner.spawn_area;
        const float half_size_x = aabb.size.x / 2.0f;
        const float half_size_y = aabb.size.y / 2.0f;
        const int rnd_x = engine::rand_det_s(rnd.rng, aabb.center.x - half_size_x, aabb.center.x + half_size_x);
        const int rnd_y = engine::rand_det_s(rnd.rng, aabb.center.y - half_size_y, aabb.center.y + half_size_y);
        set_position(r, e, { rnd_x, rnd_y });
      }

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d

// HACK: randomly spawn enemies at edges of screen
// // basically, if the camera is at 0,0
// // the edges of the screen are -width/2, width/2
// // i need x to be less than -width/2, or greater than width/2.
// static engine::RandomState rnd;
// static float cooldown = 0.5f;
// static float cooldown_left = 0.5f;
// cooldown_left -= dt;
// if (cooldown_left <= 0.0f) {
//   cooldown_left = cooldown;
//   const auto camera_e = get_first<OrthographicCamera>(r);
//   const auto camera_t = r.get<TransformComponent>(camera_e);
//   glm::ivec2 cpos = { camera_t.position.x, camera_t.position.y };
//   const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
//   const auto screen_halfsize = ri.viewport_size_render_at / 2;
//   const float left_or_right = (engine::rand_01(rnd.rng) * 2.0f) - 1.0f;
//   const float up_or_down = (engine::rand_01(rnd.rng) * 2.0f) - 1.0f;
//   const float extra = 100.0f;
//   const float pos_edge_y = cpos.y + screen_halfsize.y;
//   const float neg_edge_y = cpos.y - screen_halfsize.y;
//   const float pos_edge_x = cpos.x + screen_halfsize.x;
//   const float neg_edge_x = cpos.x - screen_halfsize.x;
//   int x_spawn_pos = 0;
//   int y_spawn_pos = 0;
//   if (up_or_down > 0.0f) // down
//     y_spawn_pos = engine::rand_det_s(rnd.rng, pos_edge_y, pos_edge_y + extra);
//   else // up
//     y_spawn_pos = engine::rand_det_s(rnd.rng, neg_edge_y, neg_edge_y - extra);
//   if (left_or_right > 0.0f) // right
//     x_spawn_pos = engine::rand_det_s(rnd.rng, pos_edge_x, pos_edge_x + extra);
//   else // left
//     x_spawn_pos = engine::rand_det_s(rnd.rng, neg_edge_x, neg_edge_x - extra);
//   const auto enemy = create_gameplay(r, EntityType::enemy_grunt);
//   auto& enemy_aabb = r.get<AABB>(enemy);
//   enemy_aabb.center = { x_spawn_pos, y_spawn_pos };
// }