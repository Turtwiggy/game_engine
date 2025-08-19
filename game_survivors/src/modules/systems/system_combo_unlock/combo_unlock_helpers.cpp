#include "pch.hpp"

#include "combo_unlock_components.hpp"
#include "combo_unlock_helpers.hpp"

#include "engine/maths/maths.hpp"

namespace game2d {

ComboUnlockComponent
generate_combo_component(entt::registry& r)
{
  static int seed = 0;
#if defined(_DEBUG)
  seed = engine::get_system_time_for_seed();
#else
  seed = engine::get_system_time_for_seed();
#endif
  static engine::RandomState rnd(seed);

  const int n_dirs = 4;
  auto generate_combo_element = [&]() -> COMBO_DIR {
    const auto idx = engine::rand_det_s(rnd.rng, (int)0, (int)n_dirs);
    return static_cast<COMBO_DIR>(idx);
  };

  ComboUnlockComponent comp;
  comp.unlock.clear();
  for (int i = 0; i < 4; i++)
    comp.unlock.push_back(generate_combo_element());
  return comp;
}

std::string
get_sprite_for_combodir(COMBO_DIR dir)
{
  if (dir == COMBO_DIR::U)
    return "ARROW_UP";

  else if (dir == COMBO_DIR::D)
    return "ARROW_DOWN";

  else if (dir == COMBO_DIR::L)
    return "ARROW_LEFT";

  else
    return "ARROW_RIGHT";
};

} // namespace game2d