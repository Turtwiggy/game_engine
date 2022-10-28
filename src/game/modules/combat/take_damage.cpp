#include "take_damage.hpp"

#include "components.hpp"

#include <entt/entt.hpp>

#include <string>

namespace game2d {

void
update_take_damage_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto& eb = game.dead;

  const auto& view = r.view<TakeDamageComponent, HealthComponent, const TagComponent>();
  for (auto [entity, damages, health, tag] : view.each()) {

    // No hits received.
    if (damages.damage.size() == 0)
      continue;

    int old_hp = health.hp;

    int base_damages = 0;
    int extra_damages = 0;
    int mitigated_damages = 0;

    // apply all damage that's meant to be taken
    for (const auto& dmg_request : damages.damage) {
      base_damages += dmg_request.base_damage;
      extra_damages += dmg_request.extra_damage;
      mitigated_damages += dmg_request.mitigated_damage;
    }

    const int final_damage = glm::max(0, base_damages + extra_damages - mitigated_damages);
    health.hp -= final_damage;

    // log the damage event
    const int damage_taken = glm::abs(health.hp - old_hp);

    std::string msg = std::to_string(damages.damage.size()) + " hits.";
    msg += std::to_string(final_damage) + " damage.";
    msg += std::to_string(mitigated_damages) + " blocked.";
    msg += std::to_string(health.hp) + " hp. ";
    game.ui_events.events.push_back(msg);

    // check for deads
    if (health.hp <= 0) {
      eb.dead.emplace(entity);
      r.emplace<IsDead>(entity);

      std::string dead_msg = tag.tag + " died.";
      game.ui_events.events.push_back(dead_msg);
    }

    damages.damage.clear();
  }
};

} // namespace game2d