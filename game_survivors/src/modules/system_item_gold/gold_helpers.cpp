#include "gold_helpers.hpp"

#include "engine/io/settings.hpp"
#include <string>

namespace game2d {

SINGLE_GoldComponent
load_gold_from_disk(entt::registry& r)
{
  SINGLE_GoldComponent gold_c;

  const auto val = gesert_string("GOLD_AMOUNT", "0");
  gold_c.amount = std::stoi(val);

  const auto info_str = std::format("You have {} gold", gold_c.amount);
  SDL_Log("%s", info_str.c_str());

  return gold_c;
};

} // namespace game2d