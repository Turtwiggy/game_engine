#include "rooms.hpp"

#include "physics/helpers.hpp"

namespace game2d {

bool
rooms_overlap(const Room& r0, const Room& r1)
{
  PhysicsTransformComponent p0;
  p0.x_tl = r0.x1;
  p0.y_tl = r0.y1;
  p0.w = r0.w;
  p0.h = r0.h;

  PhysicsTransformComponent p1;
  p1.x_tl = r1.x1;
  p1.y_tl = r1.y1;
  p1.w = r1.w;
  p1.h = r1.h;

  return collide(p0, p1);
};

} // namespace game2d