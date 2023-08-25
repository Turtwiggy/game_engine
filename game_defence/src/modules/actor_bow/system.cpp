#include "system.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"
#include <glm/common.hpp>
#include <glm/gtx/compatibility.hpp> // lerp
#include <modules/actor_player/components.hpp>

namespace game2d {

void
update_bow_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const auto& view = r.view<BowComponent, TransformComponent>();
  for (const auto& [entity, bow, bow_transform] : view.each()) {
    //
    const auto& parent = bow.parent;
    if (parent == entt::null)
      continue;
    const auto& parent_aabb = r.get<AABB>(parent);
    const auto& parent_pos = parent_aabb.center;
    const auto& player = r.get<PlayerComponent>(parent);

    // if (parent_pos != bow.target_position) {
    //   // target has moved...
    //   // bow.lerp_amount = 0;
    //   if (bow.lerp_amount >= 1.0f) {
    //     bow.lerp_amount = 0.0f;
    //     bow.target_position = parent_pos;
    //   }
    // }

    const float dt = milliseconds_dt / 1000.0f;

    const glm::vec2 start_pos = { bow_transform.position.x, bow_transform.position.y };
    const glm::vec2 target_pos = parent_pos + player.offset;

    const glm::vec2 bow_dir = player.offset;
    const float bow_angle = engine::dir_to_angle_radians(bow_dir) - engine::PI;
    bow_transform.rotation_radians.z = bow_angle;

    // todo: get player input,
    // rotate bow to that direction
    // and offset the bow
    // keys.r_analogue_x = x_axis;
    // keys.r_analogue_y = y_axis;
    // weapon_target_pos = pos + offset_pos;

    float t = bow.lerp_speed * dt;
    // t = t * t * (3.0f - 2.0f * t); // smooth step
    const auto new_pos = glm::lerp(start_pos, target_pos, t);

    bow_transform.position.x = static_cast<int>(new_pos.x);
    bow_transform.position.y = static_cast<int>(new_pos.y);
  }
}

// void
// enemy_ai::enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
// {
//   // calculate a vector ab
//   glm::vec2 ab = player.pos - obj.pos;
//   // calculate the point halfway between ab
//   glm::vec2 half_point = obj.pos + (ab / 2.0f);
//   // calculate the vector at a right angle
//   glm::vec2 normal = glm::vec2(-ab.y, ab.x);

//   // expensive(?) distance calc
//   float distance = glm::distance(obj.pos, player.pos);
//   float half_distance = distance / 2.0f;

//   // offset the midpoint via normal
//   float amplitude = half_distance * sin(glm::radians(obj.approach_theta_degrees));
//   half_point += (glm::normalize(normal) * amplitude);

//   glm::vec2 dir = glm::normalize(half_point - obj.pos);

//   move_along_vector(obj, dir, delta_time_s);
// };

} // namespace game2d