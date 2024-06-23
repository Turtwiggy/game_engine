#include "helpers.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"

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

void
set_position_grid(entt::registry& r, const entt::entity& e, const glm::ivec2 gridpos)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(gridpos, map_c.tilesize);
  const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
  const glm::ivec2 pos = worldspace + offset;
  set_position(r, e, pos);
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

void
set_colour(entt::registry& r, const entt::entity& e, const engine::SRGBColour& col)
{
  auto& sc = r.get<SpriteComponent>(e);
  sc.colour = engine::SRGBToLinear(col);
};

void
set_colour_by_tag(entt::registry& r, const entt::entity& e, const std::string& tag)
{
  const engine::SRGBColour col = get_srgb_colour_by_tag(r, tag);
  set_colour(r, e, col);
};

} // namespace game2d