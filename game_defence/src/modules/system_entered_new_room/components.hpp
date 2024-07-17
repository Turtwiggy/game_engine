#pragma once

#include "modules/gen_dungeons/components.hpp"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

struct PlayerInRoomComponent
{
  std::optional<glm::ivec2> room_tl = std::nullopt;
};

struct PlayerEnteredNewRoom
{
  entt::entity player;
  Room room;
};

} // namespace game2d