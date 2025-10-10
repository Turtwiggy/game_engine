#include "pch.hpp"

#include "scene_changed_event_components.hpp"

#include "modules/core/renderer/components.hpp"
#include "update_water_shader_helpers.hpp"

namespace game2d {

void
handle_scene_changed_event__update_water_shader(entt::registry& r, const SceneChangedEvent& evt)
{
  const auto old_s = std::string(magic_enum::enum_name(evt.old_scene));
  const auto new_s = std::string(magic_enum::enum_name(evt.new_scene));
  SDL_Log("scenechangedevent, from: %s, to: %s", old_s.c_str(), new_s.c_str());

  // GET_FIRST_OR_RETURN(SurviveTimerComponent, r, survive_e, survive_c);
  // survive_c.game_started = true;

  SINGLE_RendererInfo& ri = SINGLE_RendererInfo::instance;

  ri.water.bind();

  if (evt.new_scene == Scene::survive)
    ri.water.set_float("water_safe_radius", 800); // size of the map
  else
    ri.water.set_float("water_safe_radius", 10'000); // larger than reasonable
}

} // namespace game2d