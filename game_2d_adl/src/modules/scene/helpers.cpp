#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_level_editor/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene s)
{
  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());

  const auto& actors = r.view<EntityTypeComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(actors.begin(), actors.end());

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_LevelEditor>(r);

  // HACK: the first and only transform should be the camera
  auto& camera = get_first_component<TransformComponent>(r);
  camera.position = glm::ivec3(0, 0, 0);

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d