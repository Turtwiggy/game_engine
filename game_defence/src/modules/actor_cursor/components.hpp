#pragma once

#include <optional>

namespace game2d {

struct CursorComponent
{
  std::optional<int> grid_index = std::nullopt;

  std::vector<entt::entity> hovering_enemies;

  entt::entity click_ent = entt::null;
  entt::entity held_ent = entt::null;
  entt::entity line_ent = entt::null;
};

} // namespace game2d