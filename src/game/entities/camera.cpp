#include "camera.hpp"

#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_hierarchy/components.hpp"

namespace game2d {

static constexpr int SPRITE_SIZE = 16 * 2;

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
create_hierarchy_root_node(entt::registry& r)
{
  auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  h.root_node = r.create();

  r.emplace<TagComponent>(h.root_node, "root-node");
  r.emplace<EntityHierarchyComponent>(h.root_node, h.root_node);

  return h.root_node;
};

} // namespace game2d
