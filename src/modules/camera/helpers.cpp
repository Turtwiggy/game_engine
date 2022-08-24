// header
#include "helpers.hpp"

#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_hierarchy/components.hpp"

namespace game2d {

entt::entity
create_camera(entt::registry& r)
{
  auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();

  entt::entity e = r.create();
  r.emplace<TagComponent>(e, "camera");
  r.emplace<EntityHierarchyComponent>(e, h.root_node);
  r.emplace<TransformComponent>(e);
  r.emplace<CameraComponent>(e);

  return e;
}

entt::entity
get_main_camera(entt::registry& registry)
{
  const auto& cameras = registry.view<CameraComponent, TransformComponent>();
  const auto& main_camera = cameras.front();
  return main_camera;
};

} // namespace game2d