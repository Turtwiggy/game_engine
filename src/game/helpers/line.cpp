#include "line.hpp"

#include "engine/maths/maths.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

void
set_line(entt::registry& r, const entt::entity& e, const glm::ivec2& a, const glm::ivec2& b)
{
  //
  TransformComponent* t = r.try_get<TransformComponent>(e);
  if (t) {

    glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
    glm::vec2 nrm_dir = raw_dir;
    if (raw_dir.x != 0.0f && raw_dir.y != 0.0f)
      nrm_dir = glm::normalize(raw_dir);
    float angle = engine::dir_to_angle_radians(nrm_dir);

    t->rotation_radians.z = angle - engine::HALF_PI;
    t->scale.x = 4; // width
    t->scale.y = sqrt(pow(raw_dir.x, 2) + pow(raw_dir.y, 2));
    t->position = { (a.x + a.x + raw_dir.x) / 2, (a.y + a.y + raw_dir.y) / 2, 0 };
  }
};

} // namespace game2d