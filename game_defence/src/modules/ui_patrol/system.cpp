#include "system.hpp"

#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"

// #include "imgui.h"

namespace game2d {
using namespace std::literals;

// show worldspace ui for PatrolComponent when in range
void
update_ui_patrol_system(entt::registry& r)
{
  const auto view = r.view<PatrolComponent>();
  for (const auto& [e, p_c] : view.each()) {
    // auto& ui_txt = r.get_or_emplace<WorldspaceTextComponent>(e);
    // ui_txt.text = std::to_string(p_c.strength);

    // const auto size = get_size(r, e);
    // temporarily fixed size x, because rotating the sprite updates the aabb size

    // ui_txt.offset = { 12, 12 }; // dont cover sprite
  }
};

} // namespace game2d