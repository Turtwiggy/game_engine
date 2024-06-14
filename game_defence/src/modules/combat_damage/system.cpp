#include "system.hpp"

#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_flash_on_damage/components.hpp"
#include "modules/combat_flash_on_damage/helpers.hpp"
#include "modules/combat_powerup_doubledamage/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_knockback/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
update_take_damage_system(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<DealDamageRequest>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e_req, request] : view.each()) {

    // Does the attacker have an attack component?
    const auto* atk = r.try_get<AttackComponent>(request.from);

    // Does the defender have health?
    auto* hp = r.try_get<HealthComponent>(request.to);

    if (atk == nullptr)
      continue; // no attack damage given

    if (hp == nullptr)
      continue; // not able to take damage?

    // Was there a parent of the damage request?
    std::optional<entt::entity> parent_attacker = std::nullopt;
    if (const auto* p = r.try_get<HasParentComponent>(request.from))
      parent_attacker = p->parent;

    // Was the attacking parent a player?
    std::optional<entt::entity> player_attacker = std::nullopt;
    if (parent_attacker.has_value() && r.try_get<PlayerComponent>(parent_attacker.value()))
      player_attacker = parent_attacker.value();

    // Does the defender have the ability to be knocked back?
    auto* v = r.try_get<VelocityComponent>(request.to);
    const auto* kb = r.try_get<KnockbackComponent>(request.to);
    if (v && kb) {
      const auto atk_pos = get_position(r, request.from);
      const auto def_pos = get_position(r, request.to);
      const auto dir = engine::normalize_safe(atk_pos - def_pos);
      const float knockback_amount = 5.0F;
      v->remainder_x += -dir.x * knockback_amount;
      v->remainder_y += -dir.y * knockback_amount;
    }

    // .. show as flashing
    r.emplace_or_replace<RequestFlashComponent>(request.to);

    // .. screenshake
    const auto e = create_empty<RequestScreenshakeComponent>(r);
    r.emplace<RequestScreenshakeComponent>(e);

    // TODO: BAD. FIX.
    static engine::RandomState rnd;

    // Is this a crit?
    const int rnd_crit = int(engine::rand_det_s(rnd.rng, 0, 100));
    const bool crit = rnd_crit >= 90; // X% chance for crit
    const bool miss = rnd_crit < 5;   // X% chance to miss

    // mess with the damage
    int damage = atk->damage;
    if (crit)
      damage *= 2;
    if (miss)
      damage = 0;

    // Does the attacker have a double damage powerup?
    if (parent_attacker.has_value() && parent_attacker.value() != entt::null) {
      if (const auto* dd = r.try_get<PowerupDoubleDamage>(parent_attacker.value()))
        damage *= 2;
    }

    // .. popup some numbers as vfx
    const int base_text_separation = 7;
    int text_seperation = base_text_separation;
    const auto sprites = convert_int_to_sprites(damage);
    const auto def_transform = r.get<TransformComponent>(request.to);
    const float rnd_x = engine::rand_det_s(rnd.rng, -50, 50);
    const float rnd_y = engine::rand_det_s(rnd.rng, -50, 50);
    for (int i = 0; i < sprites.size(); i++) {
      const auto req = create_gameplay(r, EntityType::particle);

      VelocityComponent vel;
      vel.x = rnd_x;
      vel.y = rnd_y;
      r.emplace_or_replace<VelocityComponent>(req, vel);
      glm::vec3 offset_pos = def_transform.position;
      offset_pos.x += (i)*text_seperation;
      set_position(r, req, offset_pos);
      set_sprite(r, req, sprites[i]);

      // new colour
      if (crit) {
        auto& sc = r.get<SpriteComponent>(req);
        sc.colour = get_lin_colour_by_tag(r, "attack_crit");
      } else if (miss) {
        auto& sc = r.get<SpriteComponent>(req);
        sc.colour = get_lin_colour_by_tag(r, "attack_miss");
      } else {
        auto& sc = r.get<SpriteComponent>(req);
        sc.colour = get_lin_colour_by_tag(r, "attack_hit");
      }
    }

    // .. take damage
    hp->hp -= glm::max(0, damage);

    //
    if (hp->hp <= 0) {
      dead.dead.emplace(request.to);

      // If you're looking for where dead enemies drop items...
      // that's in the drop item system
    }
  }

  // done all damage requests
  r.destroy(view.begin(), view.end());
};

} // namespace game2d