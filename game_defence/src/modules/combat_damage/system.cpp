#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_flash_on_damage/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_knockback/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/ui_event_console/components.hpp"
#include "physics/components.hpp"

#include "magic_enum.hpp"

#include <algorithm>
#include <fmt/core.h>

namespace game2d {

// quip & roll a dice to say a quip.
// taking damage from a shotgun has e.g. 3 damage instances occur,
// so if we want to quip, the incoming damage must have a damage id,
// so that we know from what attack we've taken damage.
void
roll_to_quip(entt::registry& r, const entt::entity& from, const entt::entity& to)
{
  static engine::RandomState rnd;
  const auto* atk_id = r.try_get<AttackIdComponent>(from);
  const auto* atk_team = r.try_get<TeamComponent>(from);

  if (!atk_id)
    return; // incoming damage must have a damage id
  if (!atk_team)
    return; // no sentient barrels?
  if (atk_team->team != AvailableTeams::enemy)
    return; // only enemies to quip

  const int atk_unique_id = atk_id->id;
  auto& hit_list = r.get_or_emplace<DefenceHitListComponent>(to);
  auto& hit_list_attack_ids_taken = hit_list.attack_id_taken;

  auto it = std::find(hit_list_attack_ids_taken.begin(), hit_list_attack_ids_taken.end(), atk_unique_id);
  if (it != hit_list_attack_ids_taken.end()) {
    // fmt::println("atk_id already taken... still taking damage, but not rolling to quip");
    return;
  }

  // we've not taken damage from this attack id before!
  hit_list_attack_ids_taken.push_back(atk_unique_id);

  // roll to quip
  const bool should_quip = engine::rand_01(rnd.rng) < 0.05f;
  if (should_quip) {
    RequestQuip quip_req;
    quip_req.type = QuipType::TOOK_DAMAGE;
    quip_req.quipp_e = to;
    create_empty<RequestQuip>(r, quip_req);
  }
};

void
update_take_damage_system(entt::registry& r)
{
  const auto evts_e = get_first<SINGLE_EventConsoleLogComponent>(r);
  if (evts_e == entt::null)
    return;
  auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  static engine::RandomState rnd;

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

    // Does the defender have armour?
    const auto* defence = r.try_get<DefenceComponent>(request.to);

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
    create_empty<RequestScreenshakeComponent>(r);

    // .. quip
    roll_to_quip(r, request.from, request.to);

    // Is this a crit?
    const int rnd_crit = int(engine::rand_det_s(rnd.rng, 0, 100));
    const bool crit = rnd_crit >= 90; // X% chance for crit
    const bool miss = rnd_crit < 5;   // X% chance to miss

    // mess with the damage
    int damage = atk->damage;
    if (miss)
      damage = 0;

    if (defence) {
      damage -= defence->armour;
      damage = glm::max(damage, 0);
    }

    if (crit)
      damage *= 2;

    // Does the attacker have a double damage powerup?
    // if (parent_attacker.has_value() && parent_attacker.value() != entt::null) {
    //   if (const auto* dd = r.try_get<PowerupDoubleDamage>(parent_attacker.value()))
    //     damage *= 2;
    // }

    // .. take damage
    hp->hp -= glm::max(0, damage);

    const auto a_name = std::string(magic_enum::enum_name(r.get<EntityTypeComponent>(request.from).type));
    const auto b_name = std::string(magic_enum::enum_name(r.get<EntityTypeComponent>(request.to).type));
    const auto b_team = std::string(magic_enum::enum_name(r.get<TeamComponent>(request.to).team));
    const auto b_hp = std::to_string(r.get<HealthComponent>(request.to).hp);

    // log event to combat log
    const auto message = fmt::format("{} ({}) -{}HP. CUR:{}", b_name, b_team, damage, b_hp);
    evts.events.push_back(message);

    //
    if (hp->hp <= 0) {
      dead.dead.emplace(request.to);

      const auto str = fmt::format("{} died.", b_name);
      evts.events.push_back(str);

      // Make a request to spawn some particles.
      if (const auto* req = r.try_get<SpawnParticlesOnDeath>(request.to)) {
        create_empty<RequestToSpawnParticles>(r, RequestToSpawnParticles{ get_position(r, request.to) });

        // only once
        r.remove<SpawnParticlesOnDeath>(request.to);
      }

      // If you're looking for where dead enemies drop items...
      // that's in the drop item system
    }
  }

  // done all damage requests
  r.destroy(view.begin(), view.end());
};

} // namespace game2d