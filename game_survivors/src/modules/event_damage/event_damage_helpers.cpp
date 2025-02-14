#include "event_damage_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/event_death/components.hpp"
#include "modules/events/events_components.hpp"

#include <SDL2/SDL_log.h>
#include <glm/glm.hpp>
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
    int defence_amount = 0;
    // if (auto* defence_c = r.try_get<DefenceComponent>(e))
    //   defence_amount = defence_c->armour;
    amount_final -= defence_amount;
  }

  if (type == DamageType::PURE) {
    // .. pure not blocked ..
  }

  // damage shouldnt be negative
  return glm::max(amount_final, 0.0f);
};

void
handle_damage_event_take_damage(entt::registry& r, const DamageEvent& evt)
{
  auto to_e = evt.to;

  auto* hp = r.try_get<HealthComponent>(to_e);
  if (!hp) {
    const auto& tag_c = r.get<TagComponent>(to_e);
    auto err = std::format("handle_damage_event(): {} has no HealthComponent", tag_c.tag);
    SDL_Log("%s", err.c_str());
    throw std::runtime_error(err);
    return;
  }

  const float damage = calculate_damage_to_take(r, evt);

  // log evt
  const auto b_name = std::string(r.get<TagComponent>(to_e).tag);
  // const auto message = std::format("({}) damaged for {}", b_name, damage);
  // SDL_Log("%s", message.c_str());

  // apply damage
  hp->hp -= damage;

  const auto parent_e = r.get<HasParentComponent>(to_e).parent;
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