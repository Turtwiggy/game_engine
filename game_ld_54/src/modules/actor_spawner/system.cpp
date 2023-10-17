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

// #include "entt/helpers.hpp"
// #include "events/components.hpp"
// #include "events/helpers/keyboard.hpp"

namespace game2d {

// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};
// const float safe_radius_around_player = 8000.0f;

void
update_spawner_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;
  const auto& anims = get_first_component<SINGLETON_Animations>(r);

  // hack: press enter to spawn something
  // bool spawn = false;
  // const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  // if (get_key_down(input, SDL_SCANCODE_RETURN))
  //   spawn = true;

  const auto& view = r.view<const TransformComponent, SpawnerComponent, AttackCooldownComponent>();
  for (const auto& [entity, transform, spawner, cooldown] : view.each()) {

    if (cooldown.on_cooldown)
      return;

    if (spawner.continuous_spawn) {

      // must set a class for the type of unit to spawn
      if (!spawner.class_set)
        continue;

      if (spawner.enemies_to_spawn > 0)
        spawner.enemies_to_spawn--;
      else
        continue; // spawner is done

      auto e = create_gameplay(r, spawner.type_to_spawn);

      // set position for transform
      auto& t = r.get<TransformComponent>(e);
      t.position = { transform.position.x, transform.position.y, 0 };
      t.rotation_radians = transform.rotation_radians;
      if (transform.scale.x != 0 && transform.scale.y != 0)
        t.scale = transform.scale;

      // set position for aabb
      if (auto* aabb = r.try_get<AABB>(e)) {
        aabb->center = { transform.position.x, transform.position.y };
        aabb->size = { transform.scale.x, transform.scale.y };
      }

      // set sprite
      if (auto* sc = r.try_get<SpriteComponent>(e)) {
        const auto& sprite = spawner.sprite;
        const auto anim = find_animation(anims.animations, sprite);
        sc->x = anim.animation_frames[0].x;
        sc->y = anim.animation_frames[0].y;
        if (anim.angle_degrees != 0.0f)
          sc->angle_radians = glm::radians(anim.angle_degrees);
      }

      // set colour
      if (auto* scc = r.try_get<SpriteColourComponent>(e)) {
        if (spawner.colour != nullptr)
          scc->colour = spawner.colour;
      }

      // set hp
      if (auto* hp = r.try_get<HealthComponent>(e))
        hp->hp = spawner.hp;

      // CreateEntityRequest req;
      // req.type = spawner.type_to_spawn;
      // req.transform = transform; // at current pos
      // r.emplace<CreateEntityRequest>(r.create(), req);

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d