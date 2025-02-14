#include "event_damage_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_death/components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade_dodge/upgrade_dodge_components.hpp"

#include <SDL2/SDL_log.h>
#include <glm/glm.hpp>
#include <magic_enum.hpp>

#include <stdexcept>

namespace game2d {

void
additional_misc_damage_events(entt::registry& r, const entt::entity to_e)
{
  // .. pop & flash
  if (r.try_get<RequestHitScaleComponent>(to_e) == nullptr)
    r.emplace<RequestHitScaleComponent>(to_e);

  // .. screenshake
  // create_empty<RequestScreenshakeComponent>(r);
};

float
calculate_damage_to_take(entt::registry& r, const DamageEvent& evt)
{
  const auto amount = evt.amount;
  const auto type = evt.type;
  const auto e = evt.to; // Note: evt.to is a fixture

  float amount_final = amount;

  if (type == DamageType::PHYSICAL) {
    // int defence_amount = 0;
    // if (auto* defence_c = r.try_get<DefenceComponent>(e))
    //   defence_amount = defence_c->armour;
    // amount_final -= defence_amount;
  }

  if (type == DamageType::PURE) {
    // .. pure not blocked ..
  }

  // damage shouldnt be negative
  return glm::max(amount_final, 0.0f);
};

bool
check_if_dodge(entt::registry& r, entt::entity to, engine::RandomState& rnd, const StatModifierComponent& stats_c)
{
  // praise be, to RNGesus
  const int roll = engine::rand_det_s(rnd.rng, 0, 100);

  const auto dodge_val = r.get<ActorDodgeComponent>(to).dodge_percent;
  const auto dodge_key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_DODGE_CHANCE));
  const auto dodge_mod_val = stats_c.apply_modifiers(dodge_val, dodge_key); // percent.

  // your dodge percent is between 0 and anything
  // system produces a value between 0 and 100
  return roll < dodge_mod_val;
};

std::pair<bool, float>
check_if_crit(entt::registry& r, const DamageEvent& evt, engine::RandomState& rnd)
{
  // crit info would be on the evt.from (i.e. a bullet)
  if (evt.from == entt::null)
    return { false, 0.0 };

  const auto* bullet_crit_c = r.try_get<BulletCrit>(evt.from);
  if (!bullet_crit_c)
    return { false, 0.0 };

  // no chance to chrit
  if (bullet_crit_c->crit_chance <= 0.0f)
    return { false, 0.0 };

  //
  // note: crit values have already had modifiers
  // applied when the bullet was created
  //

  // praise be, to RNGesus
  const int roll = engine::rand_det_s(rnd.rng, 0, 100);

  const auto crit_chance_val = bullet_crit_c->crit_chance;
  const auto crit_damage_val = bullet_crit_c->crit_damage;
  const auto critical_hit = roll < crit_chance_val;
  const auto critical_mul = crit_damage_val / 100.0f; // convert percent to multiplier
  return { critical_hit, critical_mul };
};

void
handle_damage_event_take_damage(entt::registry& r, const DamageEvent& evt)
{
  auto to_e = evt.to;
  // note: evt.to is a fixture, not the parent with all the components on
  auto* hp = r.try_get<HealthComponent>(to_e);
  if (!hp) {
    const auto& tag_c = r.get<TagComponent>(to_e);
    auto err = std::format("handle_damage_event(): {} has no HealthComponent", tag_c.tag);
    SDL_Log("%s", err.c_str());
    throw std::runtime_error(err);
    return;
  }

  static engine::RandomState rnd(0);

  const auto parent_e = r.get<HasParentComponent>(to_e).parent;
  const auto* your_stats_c = r.try_get<StatModifierComponent>(parent_e);
  if (your_stats_c) {
    // did you dodge?
    if (check_if_dodge(r, parent_e, rnd, *your_stats_c)) {
      SDL_Log("something dodged");
      return;
    }
  }

  float damage = calculate_damage_to_take(r, evt);

  // did you crit?
  const auto [crit, crit_mul] = check_if_crit(r, evt, rnd);
  if (crit) {
    const auto info_str = std::format("something was crit with a x{:.2f} multiplier", crit_mul);
    SDL_Log("%s", info_str.c_str());
    damage *= crit_mul;
  }

  // log evt
  // const auto b_name = std::string(r.get<TagComponent>(to_e).tag);
  // const auto message = std::format("({}) damaged for {}", b_name, damage);
  // SDL_Log("%s", message.c_str());

  // apply damage
  hp->hp -= damage;

  additional_misc_damage_events(r, parent_e);

  if (hp->hp <= 0) {
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(parent_e);

    // const auto& parent_name = r.get<TagComponent>(parent_e).tag;
    // const auto str = std::format("{} died. Parent: {}", b_name, parent_name);
    // SDL_Log("%s", str.c_str());

    // Send death event.
    DeathEvent d_evt;
    d_evt.killed_by = evt.from;                            // can be entt::null
    d_evt.dead = r.get<HasParentComponent>(evt.to).parent; // parent not fixture
    auto& evts = get_first_component<SINGLE_Events>(r);
    evts.dispatcher->trigger(d_evt);
    evts.dispatcher->update();
  }
};

} // namespace game2d