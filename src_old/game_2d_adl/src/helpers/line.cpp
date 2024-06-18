#include "line.hpp"

#include "maths/maths.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

namespace game2d {

LineInfo
generate_line(const glm::ivec2& a, const glm::ivec2& b, const int width)
{
  const glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
  const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);
  const float angle = engine::dir_to_angle_radians(nrm_dir) + engine::HALF_PI;

  LineInfo line;
  line.position = { (a.x + a.x + raw_dir.x) / 2, (a.y + a.y + raw_dir.y) / 2 };
  line.rotation = angle;
  line.scale.x = width;
  line.scale.y = sqrt(pow(raw_dir.x, 2) + pow(raw_dir.y, 2));
  return line;
};

}; // namespace game2d

void
game2d::set_transform_with_line(TransformComponent& t, const LineInfo& line)
{
  t.position = { line.position.x, line.position.y, 0 };
  t.scale = { line.scale.x, line.scale.y, 0 };
  t.rotation_radians.z = line.rotation;
};

void
game2d::set_transform_with_line(entt::registry& r, const entt::entity& e, const LineInfo& li)
{
  auto& t = r.get<TransformComponent>(e);
  set_transform_with_line(t, li);
};

void
game2d::set_position_with_line(entt::registry& r, const entt::entity& e, const LineInfo& li)
{
  if (auto* aabb = r.try_get<AABB>(e)) {
    aabb->center = { li.position.x, li.position.y };
    aabb->size = { li.scale.x, li.scale.y };
  }
  set_transform_with_line(r, e, li);
}
