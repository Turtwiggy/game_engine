#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct PlayerInRoomComponent
{
  entt::entity room_e = entt::null;
};

struct PlayerEnteredNewRoom
{
  entt::entity player = entt::null;
  entt::entity room_e = entt::null;
};

} // namespace game2d