#include "collisions.hpp"

#include "modules/gen_dungeons/components.hpp"

namespace game2d {

bool
collide(const RoomAABB& one, const RoomAABB& two)
{
  const glm::ivec2 one_tl_pos = { one.center.x - one.size.x / 2, one.center.y - one.size.y / 2 };
  const glm::ivec2 two_tl_pos = { two.center.x - two.size.x / 2, two.center.y - two.size.y / 2 };

  // collision x-axis?
  bool collision_x = one_tl_pos.x + one.size.x > two_tl_pos.x && two_tl_pos.x + two.size.x > one_tl_pos.x;

  // collision y-axis?
  bool collision_y = one_tl_pos.y + one.size.y > two_tl_pos.y && two_tl_pos.y + two.size.y > one_tl_pos.y;

  return collision_x && collision_y;
};

} // namespace game2d