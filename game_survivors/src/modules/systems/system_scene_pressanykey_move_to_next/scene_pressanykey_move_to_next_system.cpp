#include "scene_pressanykey_move_to_next_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/scene/scene_helpers.hpp"

namespace game2d {

void
update_scene_pressanykey_move_to_next_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  // handle press anykey.

  if (!input.unprocessed_inputs.empty())
    move_to_scene_start(r, Scene::menu);
};

} // namespace game2d