#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

enum class SpaceshipDoorState
{
  OPEN,
  CLOSED,
  BETWIXT,
};

struct SpaceshipDoorComponent
{
  glm::vec2 closed_size{ 0, 0 };
  glm::vec2 to_close_increment{ 0.0f, 0.0f };

  SpaceshipDoorState state = SpaceshipDoorState::CLOSED;
  entt::entity parent_room = entt::null;
};

enum class PressurePlateType
{
  OPEN,
  CLOSE,
};

struct SpaceshipPressureplateComponent
{
  entt::entity door = entt::null;
  PressurePlateType type = PressurePlateType::OPEN;
};

} // namespace game2d