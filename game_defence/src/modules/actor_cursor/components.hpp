#pragma once

#include <optional>

namespace game2d {

struct CursorComponent
{
  std::optional<int> grid_index = std::nullopt;

  // std::vector<entt::entity> hovering_enemies;

  // entt::entity click_ent = entt::null;
  // entt::entity held_ent = entt::null;
  // entt::entity line_ent = entt::null;

  // debug dda
  // entt::entity dda_start = entt::null;
  // entt::entity dda_intersection = entt::null;
  // entt::entity dda_end = entt::null;
};

} // namespace game2d