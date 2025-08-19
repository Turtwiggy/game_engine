#pragma once

#include <entt/fwd.hpp>

namespace game2d {

enum class COMBO_DIR
{
  U,
  D,
  L,
  R
};

struct ComboUnlockComponent
{
  std::vector<COMBO_DIR> unlock{
    COMBO_DIR::U,
    COMBO_DIR::D,
    COMBO_DIR::L,
    COMBO_DIR::R,
  };
  std::vector<COMBO_DIR> current;

  bool display = false;
};

} // namespace game2d