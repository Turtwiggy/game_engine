#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"

#include <algorithm>
#include <glm/common.hpp>
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_bow_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;
  const float bullet_speed = 30.0f;

  auto& lifecycle = get_first_component<SINGLETON_EntityBinComponent>(r);
  const auto& created = lifecycle.created_this_frame;

  const auto& view = r.view<BowComponent, TransformComponent, HasParentComponent>();
  for (const auto& [entity, bow, bow_transform, bow_parent] : view.each()) {
    //
    const auto& parent = bow_parent.parent;
    if (parent == entt::null)
      continue;
    const auto& [player, player_aabb, input] = r.get<const PlayerComponent, const AABB, InputComponent>(parent);
    const auto& parent_pos = player_aabb.center;

    glm::vec2 r_nrm_dir = { input.rx, input.ry };
    if (r_nrm_dir.x != 0.0f || r_nrm_dir.y != 0.0f)
      r_nrm_dir = glm::normalize(r_nrm_dir);
    const glm::ivec2 offset = { r_nrm_dir.x * bow.offset, r_nrm_dir.y * bow.offset };

    const glm::vec2 start_pos = { bow_transform.position.x, bow_transform.position.y };
    const glm::vec2 target_pos = parent_pos + offset;

    // note: this lerp would not work in an update() function where dt is variable
    // const float t = dt * bow.lerp_speed;
    const float t = 1.0f - glm::pow(0.5f, dt * bow.lerp_speed); // ?
    const auto new_pos = glm::lerp(start_pos, target_pos, t);

    bow_transform.position.x = static_cast<int>(new_pos.x);
    bow_transform.position.y = static_cast<int>(new_pos.y);

    const float bow_angle = engine::dir_to_angle_radians(offset) - engine::PI;
    bow_transform.rotation_radians.z = bow_angle;

    //
    // prime the arrow
    //

    if (bow.time_between_bullets_left > 0.0f) // cooldown
      bow.time_between_bullets_left -= dt;

    bool is_aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;

    bool allowed_to_shoot = true;
    allowed_to_shoot &= bow.time_between_bullets_left <= 0.0f;
    allowed_to_shoot &= is_aiming;
    allowed_to_shoot &= !bow.in_windup; // hold one arrow at a time

    if (input.shoot && allowed_to_shoot) {

      CreateEntityRequest req;
      req.type = EntityType::actor_arrow;
      req.position = bow_transform.position;
      req.parent = entity; // set arrow's parent as the bow
      r.emplace<CreateEntityRequest>(r.create(), req);

      // request audio
      // r.emplace<AudioRequestPlayEvent>(r.create(), "SHOOT_01");

      // reset shot cooldown timer
      bow.time_between_bullets_left = bow.time_between_bullets;

      bow.in_windup = true;
    }

    //
    // release the arrow
    //

    // grab the created arrows for this bow
    // (if the bow was the parent)
    std::copy_if(created.begin(), created.end(), std::back_inserter(bow.my_arrows), [&r, &entity](const entt::entity& e) {
      const auto* p = r.try_get<HasParentComponent>(e);
      return p && p->parent == entity;
    });

    auto it = bow.my_arrows.begin();
    while (it != bow.my_arrows.end()) {

      const auto& val = (*it);
      if (!r.valid(val)) {
        std::cout << "arrow became invalid?" << std::endl;
        it = bow.my_arrows.erase(it);
        bow.in_windup = false;
        continue;
      }

      EntityType t = r.get<EntityTypeComponent>(val).type;
      auto& arrow = r.get<ArrowComponent>(val);

      // we have an arrow in the windup phase...
      auto& arrow_aabb = r.get<AABB>(val);
      arrow_aabb.center = { bow_transform.position.x, bow_transform.position.y };
      auto& arrow_transform = r.get<TransformComponent>(val);
      arrow_transform.rotation_radians.z = bow_angle;

      if (!input.shoot) // assume shoot was released....
      {
        bow.in_windup = false;

        if (!is_aiming) {
          // released shoot but not aiming anymore!
          // cancel the arrow
          std::cout << "arrow cancelled" << std::endl;
          lifecycle.dead.emplace(val);
          it = bow.my_arrows.erase(it);
          continue;
        }

        // Turn the arrow Live!
        r.emplace<AttackComponent>(val, 3);

        auto& arrow_vel = r.get<VelocityComponent>(val);
        arrow_vel.x = r_nrm_dir.x * bullet_speed;
        arrow_vel.y = r_nrm_dir.y * bullet_speed;

        std::cout << "arrow released" << std::endl;
        it = bow.my_arrows.erase(it);
        continue;
      }

      // next
      ++it;
    }
  }
}

} // namespace game2d