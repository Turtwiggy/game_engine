#include "event_damage_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "modules/combat/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_quips/components.hpp"

#include <fmt/core.h>
#include <glm/glm.hpp>

namespace game2d {

void
additional_misc_damage_events(entt::registry& r, const entt::entity to_e)
{
  // .. show as flashing
  // r.emplace_or_replace<RequestFlashComponent>(request.to);

  // .. screenshake
  create_empty<RequestScreenshakeComponent>(r);

  // roll_to_quip()
  static engine::RandomState rnd;
  const bool should_quip = engine::rand_01(rnd.rng) < 0.05f;
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

void
handle_damage_event(entt::registry& r, const DamageEvent& evt)
{
  const auto from_e = evt.from; // previously bullet, now player?
  const auto to_e = evt.to;
  const auto amount = evt.amount;

  auto* hp = r.try_get<HealthComponent>(to_e);
  if (!hp) {
    fmt::println("handle_damage_event(): to_e has no HealthComponent");
    return;
  }

  int defence_amount = 0;
  if (auto* defence_c = r.try_get<DefenceComponent>(to_e))
    defence_amount = defence_c->armour;

  float damage = amount;
  damage -= defence_amount;
  damage = glm::max(damage, 0.0f);

  const auto a_name = std::string(r.get<TagComponent>(from_e).tag);
  const auto b_name = std::string(r.get<TagComponent>(to_e).tag);
  const auto message = fmt::format("({}) atk ({}) for {}", a_name, b_name, damage);
  fmt::println("{}", message);

  // .. take damage
  hp->hp -= static_cast<int>(glm::max(0.0f, damage));
  additional_misc_damage_events(r, to_e);

  if (hp->hp <= 0) {
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(to_e);

    const auto str = fmt::format("{} died.", b_name);
    fmt::println("{}", str);
    additional_misc_death_events(r, to_e);
  }
};

} // namespace game2d