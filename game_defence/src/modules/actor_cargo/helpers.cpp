#include "modules/actor_cargo/helpers.hpp"

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

} // namespace game2d