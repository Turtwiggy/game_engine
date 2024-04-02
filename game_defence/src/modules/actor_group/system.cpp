#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_group/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/algorithm_dda/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"


#include <glm/glm.hpp>

#include <map>
#include <vector>

namespace game2d {

void
update_actor_group_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  // identify groups
  //
  std::map<entt::entity, std::vector<entt::entity>> groups;
  const auto& part_of_group_view =
    r.view<const PartOfGroupComponent, const HasParentComponent, const HasTargetPositionComponent>();
  for (const auto& [e, part_of_group, parent, has_target] : part_of_group_view.each()) {
    groups[parent.parent].push_back(e);
  }

  // Apply group behaviour
  //
  for (const auto& group : groups) {
    //
    // parent info
    //
    const auto& aabb = r.get<AABB>(group.first);

    // work out some group info
    //
    const int total_targets = group.second.size();
    int total_length = 0;
    for (const auto& e : group.second)
      total_length += r.get<AABB>(e).size.x;

    // set group-unit position based on info
    //
    const float size_of_each_unit = total_length / float(total_targets);
    const glm::vec2 size{ size_of_each_unit, size_of_each_unit };
    const glm::vec2 total_targets_vec{ total_targets, total_targets };
    for (int i = 1; const auto& e : group.second) {
      auto& target = r.get<HasTargetPositionComponent>(e);
      target.position = aabb.center;

      const glm::vec2 raw_dir = mouse_pos - aabb.center;
      const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);
      const glm::vec2 perp_dir = { -nrm_dir.y, nrm_dir.x };
      target.position += perp_dir * ((glm::vec2(i, i) * size) - (size / 2.0f) - (size * total_targets_vec / 2.0f));

      i++;
    }
  }
};

} // namespace game2d