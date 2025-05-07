#include "pch.hpp"

#include "gold_helpers.hpp"

#include "modules/core/io/io_helpers.hpp"

namespace game2d {

SINGLE_GoldComponent
load_gold_from_disk(entt::registry& r)
{
  SINGLE_GoldComponent gold_c;

  const auto val_opt = savefile_get_key(r, "GOLD_AMOUNT");
  if (!val_opt.has_value())
    return gold_c;
  const auto val_json = val_opt.value();

  int gold = 0;
  val_json.get_to<int>(gold);

  const auto info_str = std::format("You have {} gold", gold_c.amount);
  SDL_Log("%s", info_str.c_str());

  gold_c.amount = gold;
  return gold_c;
};

} // namespace game2d