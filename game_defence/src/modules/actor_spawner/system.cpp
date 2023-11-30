#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};
// const float safe_radius_around_player = 8000.0f;

// should be in something
static const float time_between_spawns = 2.0f;
static float time_between_spawns_left = time_between_spawns;

void
update_spawner_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view =
    r.view<const TransformComponent, SpawnerComponent, AttackCooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, spawner, cooldown] : view.each()) {

    if (cooldown.on_cooldown)
      return;

    if (spawner.continuous_spawn) {

      // auto e = create_gameplay(r, spawner.type_to_spawn);

      // // set position for transform
      // auto& t = r.get<TransformComponent>(e);
      // t.position = { transform.position.x, transform.position.y, 0 };
      // t.rotation_radians = transform.rotation_radians;
      // if (transform.scale.x != 0 && transform.scale.y != 0)
      //   t.scale = transform.scale;

      // // set position for aabb
      // if (auto* aabb = r.try_get<AABB>(e)) {
      //   aabb->center = { transform.position.x, transform.position.y };
      //   aabb->size = { transform.scale.x, transform.scale.y };
      // }

      // // set sprite
      // if (auto* sc = r.try_get<SpriteComponent>(e)) {
      //   const auto& sprite = spawner.sprite;
      //   const auto anim = find_animation(anims.animations, sprite);
      //   sc->x = anim.animation_frames[0].x;
      //   sc->y = anim.animation_frames[0].y;
      //   if (anim.angle_degrees != 0.0f)
      //     sc->angle_radians = glm::radians(anim.angle_degrees);
      // }

      // // set colour
      // if (auto* scc = r.try_get<SpriteColourComponent>(e)) {
      //   if (spawner.colour != nullptr)
      //     scc->colour = spawner.colour;
      // }

      // // set hp
      // if (auto* hp = r.try_get<HealthComponent>(e))
      //   hp->hp = spawner.hp;

      // TODO: fix this
      // const auto req = create_gameplay(r, spawner.type_to_spawn);
      // r.get<TransformComponent>(req).position = transform.position;

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d