#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "modules/pathfinding_flowfield/pathfinding_flowfield_components.hpp"

namespace game2d {

namespace tests {

TEST(TestSuite, FlowField_Center)
{
  // arrange
  entt::registry r;
  MapComponent map_c;
  map_c.tilesize = 25.0f;
  map_c.xmax = 10;
  map_c.ymax = map_c.xmax;
  map_c.map.resize(map_c.xmax * map_c.ymax);
  create_empty<MapComponent>(r, map_c);
  create_empty<SINGLE_Flowfield>(r);

  // act
  // auto flowfield = generate_flowfield(r, )

  // assert
}

} // namespace tests

} // namespace game2d