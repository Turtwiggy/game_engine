#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <algorithm>
#include <glm/common.hpp>
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

//
// Bow:
// click: create arrow
// release: releases arrow
//

void
update_bow_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;
  auto& lifecycle = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<BowComponent, HasParentComponent, AttackCooldownComponent, const TransformComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, bow, parent, cooldown, transform] : view.each()) {
    const auto& p = parent.parent;
    if (p == entt::null)
      continue;
    const auto& [player, input] = r.get<PlayerComponent, InputComponent>(p);

    //
    // windup the arrow
    //

    glm::vec2 r_nrm_dir = { input.rx, input.ry };
    if (r_nrm_dir.x != 0.0f || r_nrm_dir.y != 0.0f)
      r_nrm_dir = glm::normalize(r_nrm_dir);
    const glm::ivec2 offset = { r_nrm_dir.x * player.weapon_offset, r_nrm_dir.y * player.weapon_offset };

    bool aiming = glm::abs(input.rx) > 0.3f || glm::abs(input.ry) > 0.3f;

    bool allowed_to_shoot = !cooldown.on_cooldown;
    allowed_to_shoot &= aiming;         // must be aiming
    allowed_to_shoot &= !bow.in_windup; // not already held

    if (input.shoot && allowed_to_shoot) {

      const auto req = create_gameplay(r, EntityType::bullet_bow);
      r.get<TransformComponent>(req).position = transform.position;
      r.get_or_emplace<HasParentComponent>(req).parent = entity;
      bow.arrows.push_back(req);

      // why bother resetting the cooldown if the bow is in windup?
      // incase spam hold & unhold button
      reset_cooldown(cooldown);

      bow.in_windup = true;
    }

    //
    // release the arrows
    //

    auto it = bow.arrows.begin();
    while (it != bow.arrows.end()) {

      const auto& val = (*it);
      if (!r.valid(val)) {
        std::cout << "arrow became invalid?" << std::endl;
        it = bow.arrows.erase(it);
        bow.in_windup = false;
        continue;
      }

      // get windup arrow to follow bow
      auto& arrow_aabb = r.get<AABB>(val);
      arrow_aabb.center = { transform.position.x, transform.position.y };
      auto& arrow_transform = r.get<TransformComponent>(val);
      arrow_transform.rotation_radians.z = transform.rotation_radians.z;

      if (!input.shoot) // arrow released
      {
        bow.in_windup = false;

        if (!aiming) {
          // released shoot but not aiming anymore!
          // cancel the arrow
          std::cout << "arrow cancelled" << std::endl;
          lifecycle.dead.emplace(val);
          it = bow.arrows.erase(it);
          continue;
        }

        // Turn the arrow Live!
        r.emplace<AttackComponent>(val, 3);
        r.emplace<EntityTimedLifecycle>(val);

        auto& arrow_vel = r.get<VelocityComponent>(val);
        arrow_vel.x = r_nrm_dir.x * bow.bow_bullet_speed;
        arrow_vel.y = r_nrm_dir.y * bow.bow_bullet_speed;

        std::cout << "arrow released" << std::endl;
        it = bow.arrows.erase(it);
        continue;
      }

      // next
      ++it;
    }
  }
};

} // namespace game2d