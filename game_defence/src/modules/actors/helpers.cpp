#include "helpers.hpp"

#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

glm::vec2
get_position(entt::registry& r, const entt::entity& e)
{
  if (const auto* aabb = r.try_get<AABB>(e))
    return aabb->center;
  const auto& t = r.get<TransformComponent>(e);
  return { t.position.x, t.position.y };
};

void
set_position(entt::registry& r, const entt::entity& e, const glm::ivec2& pos)
{
  if (auto* aabb = r.try_get<AABB>(e))
    aabb->center = pos;
  r.get<TransformComponent>(e).position = { pos.x, pos.y, 0.0f };
}

glm::ivec2
get_size(entt::registry& r, const entt::entity& e)
{
  if (auto* aabb = r.try_get<AABB>(e))
    return aabb->size;
  const auto& transform = r.get<TransformComponent>(e);
  return { transform.scale.x, transform.scale.y };
};

void
set_size(entt::registry& r, const entt::entity& e, const glm::ivec2& size)
{
  if (auto* aabb = r.try_get<AABB>(e))
    aabb->size = size;
  auto& transform = r.get<TransformComponent>(e);
  transform.scale.x = size.x;
  transform.scale.y = size.y;
};

} // namespace game2d