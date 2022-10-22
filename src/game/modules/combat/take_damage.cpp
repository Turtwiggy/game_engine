#include "take_damage.hpp"

#include "components.hpp"

#include <entt/entt.hpp>

#include <format>
#include <string>

namespace game2d {

void
update_take_damage_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto& eb = game.dead;

  const auto& view = r.view<TakeDamageComponent, HealthComponent, const TagComponent>();
  for (auto [entity, damages, health, tag] : view.each()) {

    int old_hp = health.hp;

    // apply all damage that's meant to be taken
    for (const auto& damage : damages.damage) {
      health.hp -= damage;
    }

    // log the damage event
    const int damage_taken = glm::abs(health.hp - old_hp);
    if (damage_taken > 0) {
      const std::string msg = std::format(
        "{} hits. {} damage. new_hp: {}, ({})", damages.damage.size(), damage_taken, health.hp, tag.tag.c_str());
      game.ui_events.events.push_back(msg);
    }

    // check for deads
    if (health.hp <= 0) {
      eb.dead.emplace(entity);
      r.emplace<IsDead>(entity);
      game.ui_events.events.push_back(std::format("{} died.", tag.tag.c_str()));
    }

    damages.damage.clear();
  }
};

} // namespace game2d