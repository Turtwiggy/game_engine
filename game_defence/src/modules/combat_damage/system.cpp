#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_flash_on_damage/components.hpp"
#include "modules/combat_flash_on_damage/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"
#include "renderer/helpers.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
update_take_damage_system(entt::registry& r)
{
  const auto& view = r.view<DealDamageRequest>(entt::exclude<WaitForInitComponent>);
  for (auto [e_req, request] : view.each()) {

    // Does the attacker have an attack component?
    const auto* atk = r.try_get<AttackComponent>(request.from);

    // Does the defender have health?
    auto* hp = r.try_get<HealthComponent>(request.to);

    if (!atk)
      continue; // no attack damage given
    if (!hp)
      continue; // not able to take damage?

    // .. show flash
    r.emplace_or_replace<FlashOnDamageComponent>(request.to);

    // .. play audio
    // r.emplace<AudioRequestPlayEvent>(r.create(), "HIT");

    // .. popup some numbers as vfx
    const int text_seperation = 6;
    const auto sprites = convert_int_to_sprites(atk->damage);
    const auto def_pos = r.get<TransformComponent>(request.to).position;
    for (int i = 0; i < sprites.size(); i++) {

      const auto req = create_gameplay(r, EntityType::particle);

      VelocityComponent vel;
      vel.x = 0;
      vel.y = 10; // travel up
      r.emplace_or_replace<VelocityComponent>(req, vel);

      glm::vec3 offset_pos = def_pos;
      offset_pos.x += (i + 1) * text_seperation;
      set_position(r, req, offset_pos);
      set_sprite(r, req, sprites[i]);
    }

    // .. take damage
    hp->hp -= glm::max(0, atk->damage);
  }

  // done all damage requests
  r.destroy(view.begin(), view.end());

  //
  // check if anything is dead
  //
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  for (const auto& [e, hp] : r.view<const HealthComponent>(entt::exclude<WaitForInitComponent>).each()) {
    if (hp.hp <= 0)
      dead.dead.emplace(e);
  }
};

} // namespace game2d