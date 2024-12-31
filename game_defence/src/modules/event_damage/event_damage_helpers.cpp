#include "event_damage_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/event_death/event_death_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_quips/components.hpp"

#include <SDL2/SDL_log.h>
#include <glm/glm.hpp>

#include <format>

namespace game2d {

void
additional_misc_damage_events(entt::registry& r, const entt::entity to_e)
{
  // .. pop & flash
  if (r.try_get<RequestHitScaleComponent>(to_e) == nullptr)
    r.emplace<RequestHitScaleComponent>(to_e);

  // .. screenshake
  create_empty<RequestScreenshakeComponent>(r);

  // roll_to_quip()
  static engine::RandomState rnd;
  const bool should_quip = engine::rand_01(rnd.rng) < 0.08f;
  if (should_quip) {
    RequestQuip quip_req;
    quip_req.type = QuipType::TOOK_DAMAGE;
    quip_req.quipp_e = to_e;
    create_empty<RequestQuip>(r, quip_req);
  }
};

void
additional_misc_death_events(entt::registry& r, const entt::entity to_e)
{
  if (const auto* req = r.try_get<SpawnParticlesOnDeath>(to_e)) {
    create_empty<RequestToSpawnParticles>(r, RequestToSpawnParticles{ get_position(r, to_e) });
    r.remove<SpawnParticlesOnDeath>(to_e);
  }
};

int
calculate_damage_to_take(entt::registry& r, entt::entity e, int amount, const DamageType& type)
{
  int amount_final = amount;

  if (type == DamageType::PHYSICAL) {
    int defence_amount = 0;
    if (auto* defence_c = r.try_get<DefenceComponent>(e))
      defence_amount = defence_c->armour;
    amount_final -= defence_amount;
  }

  if (type == DamageType::PURE) {
    // .. pure not blocked ..
  }

  // damage shouldnt be negative
  return glm::max(amount_final, 0);
};

void
handle_damage_event_take_damage(entt::registry& r, const DamageEvent& evt)
{
  const auto to_e = evt.to;

  auto* hp = r.try_get<HealthComponent>(to_e);
  if (!hp) {
    SDL_Log("handle_damage_event(): to_e has no HealthComponent");
    return;
  }

  const int damage = calculate_damage_to_take(r, evt.to, evt.amount, evt.type);

  // log evt
  const auto b_name = std::string(r.get<NameComponent>(to_e).name);
  const auto message = std::format("({}) damaged for {}", b_name, damage);
  SDL_Log("%s", message.c_str());

  // apply damage
  hp->hp -= damage;
  additional_misc_damage_events(r, to_e);

  if (hp->hp <= 0) {
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(to_e);

    const auto str = std::format("{} died.", b_name);
    SDL_Log("%s", str.c_str());

    // Send death event.
    auto& evts = get_first_component<SINGLE_Events>(r);
    DeathEvent evt;
    evt.dead = to_e;
    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();

    additional_misc_death_events(r, to_e);
  }
};

} // namespace game2d