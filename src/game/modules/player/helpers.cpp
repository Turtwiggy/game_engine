#include "helpers.hpp"

#include "engine/maths/grid.hpp"
#include "game/components/actors.hpp"
#include "game/modules/ai/helpers.hpp"
#include "game/modules/items/components.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/helpers/mouse.hpp"

#include <glm/glm.hpp>

namespace game2d {

std::pair<int, int>
next_dir_along_path(const std::vector<vec2i>& path)
{
  if (path.size() > 1) {
    const auto& from = path[0]; // path[0] is current
    const auto& to = path[1];
    int dx = to.x - from.x;
    int dy = to.y - from.y;
    return { dx, dy };
  }
  return { 0, 0 };
}

void
pathfind_unit_to_mouse_position(GameEditor& editor, Game& game, const entt::entity& unit)
{
  auto& r = game.state;
  const int GRID_SIZE = 16;
  const auto offset = glm::ivec2(GRID_SIZE / 2, GRID_SIZE / 2);
  const auto mouse_position = mouse_position_in_worldspace(editor, game) + offset;
  const auto mouse_grid = engine::grid::world_space_to_grid_space(mouse_position, GRID_SIZE);
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const auto& group = r.group<GridComponent, PathfindableComponent>();
  auto& transform = r.get<TransformComponent>(unit);

  // position
  const auto grid = engine::grid::world_space_to_grid_space({ transform.position.x, transform.position.y }, GRID_SIZE);
  vec2i from = { glm::clamp(grid.x, 0, d.width - 1), glm::clamp(grid.y, 0, d.height - 1) };

  // mouse position
  vec2i to = { glm::clamp(mouse_grid.x, 0, d.width - 1), glm::clamp(mouse_grid.y, 0, d.height - 1) };

  // pathfind to location
  const auto path = astar(r, from, to);

  // Set new destination
  FollowPathComponent* potential_path = r.try_get<FollowPathComponent>(unit);
  if (potential_path) {
    potential_path->calculated_path.clear();
    potential_path->calculated_path = path;
  } else {
    FollowPathComponent& new_path = r.emplace<FollowPathComponent>(unit);
    new_path.calculated_path = path;
  }
};

void
shoot(GameEditor& editor, Game& game, const entt::entity& player)
{
  auto& r = game.state;
  const auto mouse_position = mouse_position_in_worldspace(editor, game);
  auto& transform = r.get<TransformComponent>(player);

  const float bullet_speed = 50.0f;
  entt::entity bullet = create_gameplay(editor, game, EntityType::bolt);
  r.remove<AbleToBePickedUp>(bullet);
  create_renderable(editor, r, bullet, EntityType::bolt);

  glm::vec2 dir = { mouse_position.x - transform.position.x, mouse_position.y - transform.position.y };
  if (dir.x != 0.0f && dir.y != 0.0f)
    dir = glm::normalize(dir);

  TransformComponent& bullet_transform = r.get<TransformComponent>(bullet);
  bullet_transform.position = transform.position;

  VelocityComponent& vel = r.get<VelocityComponent>(bullet);
  vel.x = dir.x * bullet_speed;
  vel.y = dir.y * bullet_speed;

  float angle = engine::dir_to_angle_radians(dir);
  bullet_transform.rotation_radians.z = angle + (engine::PI / 4.0f);

  // add some randomness to the bullet's direction
  // todo: replace complete randomness with a recoil factor.
  // constexpr float angle_min_max = 10.0f * engine::PI / 180.0f;
  // const float random_angle = engine::rand_det_s(rnd.rng, -angle_min_max, angle_min_max);
  // Create an attack ID
  // std::cout << "bullet attack, attack id: " << a.id << "\n";
  // Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
  // gs.attacks.push_back(a);
};

} // namespace game2d