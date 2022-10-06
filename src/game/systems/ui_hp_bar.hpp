#pragma once

#include "game/components/app.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct HpBar
{
  entt::entity entity;
  entt::entity front;
  entt::entity back;
};

void
init_ui_hp_bar(GameEditor& editor, Game& game, entt::entity& player);

void
update_ui_hp_bar(GameEditor& editor, Game& game);

} // namespace game2d