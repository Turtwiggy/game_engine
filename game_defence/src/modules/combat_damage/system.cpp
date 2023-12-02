#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_flash_on_damage/components.hpp"
#include "modules/combat_flash_on_damage/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"
#include "renderer/helpers.hpp"
#include "sprites/helpers.hpp"

#include "maths/maths.hpp"

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
    const auto def_transform = r.get<TransformComponent>(request.to);
    for (int i = 0; i < sprites.size(); i++) {

      const auto req = create_gameplay(r, EntityType::particle);

      static engine::RandomState rnd;
      const float rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
      const float rnd_y = engine::rand_det_s(rnd.rng, -50, 50);

      VelocityComponent vel;
      vel.x = rnd_x;
      vel.y = rnd_y;
      r.emplace_or_replace<VelocityComponent>(req, vel);

      glm::vec3 offset_pos = def_transform.position;
      offset_pos.x += (i)*text_seperation;
      // offset_pos.y -= def_transform.scale.y; // show above unit
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
  // note: this is done in the respawn system
  // auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  // for (const auto& [e, hp] : r.view<const HealthComponent>(entt::exclude<WaitForInitComponent>).each()) {
  //   if (hp.hp <= 0)
  //     dead.dead.emplace(e);
  // }
};

} // namespace game2d