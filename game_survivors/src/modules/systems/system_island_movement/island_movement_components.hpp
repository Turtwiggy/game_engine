#pragma once

namespace game2d {

struct MovementIslandComponent
{
  entt::entity island_e = entt::null;
  entt::entity boat_e = entt::null;
};

struct WantToReturnToBoat
{
  bool placeholder = true;
};

struct IslandHiddenComponent
{
  entt::entity island_popup_e = entt::null;
};

} // namespace game2d