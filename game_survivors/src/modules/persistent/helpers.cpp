#include "helpers.hpp"

#include "engine/io/settings.hpp"

#include <glm/glm.hpp>

namespace game2d {

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