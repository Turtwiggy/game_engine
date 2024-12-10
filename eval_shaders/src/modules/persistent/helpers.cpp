#include "helpers.hpp"

#include "engine/io/settings.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
increment_cargo(entt::registry& r)
{
  // save to a text file the number of ships cleared. (+1, the one you just beat)
  const auto spaceship_opt = get_string(SPACESHIP_COUNT);
  const int spaceship_val = spaceship_opt.has_value() ? (std::stoi(spaceship_opt.value()) + 1) : 1;
  save_string(SPACESHIP_COUNT, std::to_string(spaceship_val));
}

void
decrement_cargo(entt::registry& r)
{
  // save to a text file the number of ships cleared. (+1, the one you just beat)
  const auto spaceship_opt = get_string(SPACESHIP_COUNT);

  int spaceship_val = spaceship_opt.has_value() ? (std::stoi(spaceship_opt.value()) - 1) : 0;
  spaceship_val = glm::max(0, spaceship_val);

  save_string(SPACESHIP_COUNT, std::to_string(spaceship_val));
}

void
save_level(entt::registry& r, int level, bool complete)
{
  save_string("level" + std::to_string(level), complete ? "true" : "false");
};

bool
get_level_complete(entt::registry& r, int level)
{
  auto str = gesert_string("level" + std::to_string(level), "false");
  return str == "true";
};

} // namespace game2d