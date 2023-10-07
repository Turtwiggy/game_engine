#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_flash_on_damage/helpers.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/procedural/cell_automata.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene s)
{
  {
    // entt::registry new_r;
    // transfer<SINGLETON_Animations>(r, new_r);
    // transfer<SINGLETON_Textures>(r, new_r);
    // transfer<SINGLETON_AudioComponent>(r, new_r);
    // transfer<SINGLETON_RendererInfo>(r, new_r);
    // transfer<SINGLETON_InputComponent>(r, new_r);
    // transfer<SINGLETON_FixedUpdateInputHistory>(r, new_r);
    // transfer<AudioSource>(r, new_r); // multiple of these
    // r = std::move(new_r);
  }

  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());

  const auto& actors = r.view<EntityTypeComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(actors.begin(), actors.end());

  const auto& reqs = r.view<CreateEntityRequest>();
  r.destroy(reqs.begin(), reqs.end());

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  destroy_and_create<SINGLETON_Wave>(r);

  const auto cursor = create_gameplay(r, EntityType::cursor);

  // Hack: create levels in code
  int level = 0;
  if (level == 0 && s == Scene::game) {
    GridComponent grid;
    grid.width = 8;
    grid.height = 8;
    grid.grid.resize(grid.width * grid.height);

    // create a spawner
    {
      const glm::ivec2 grid_pos{ 0, 0 };
      const auto grid_idx = engine::grid::grid_position_to_index(grid_pos, grid.width);
      const auto pos = engine::grid::grid_space_to_world_space(grid_pos, grid.size);
      const auto e = create_gameplay(r, EntityType::actor_spawner);
      // auto& t = r.get<AABB>(e);
      // t.center = pos;
      auto& t = r.get<TransformComponent>(e);
      t.position = { pos.x, pos.y, 0.0 };

      grid.grid[grid_idx].push_back(e);
    }

    // create a hearth
    const glm::ivec2 hearth_pos{ grid.width - 1, grid.height - 1 };
    const auto hearth_idx = engine::grid::grid_position_to_index(hearth_pos, grid.width);
    {
      const auto pos = engine::grid::grid_space_to_world_space(hearth_pos, grid.size);
      const auto e = create_gameplay(r, EntityType::actor_hearth);
      auto& t = r.get<AABB>(e);
      t.center = pos;

      grid.grid[hearth_idx].push_back(e);
    }

    // create points on grid
    auto& anims = get_first_component<SINGLETON_Animations>(r);
    for (int xy = 0; xy < grid.width * grid.height; xy++) {
      const auto gpos = engine::grid::index_to_grid_position(xy, grid.width, grid.height);
      const auto sprite = create_gameplay(r, EntityType::empty);
      // set position
      auto& spr_t = r.get<TransformComponent>(sprite);
      spr_t.position = { gpos.x * grid.size, gpos.y * grid.size, 0.0 };
      spr_t.scale = { 4, 4, 1 };
    }

    // grid.flow_field = generate_flow_field(r, grid, hearth_idx);
    r.emplace<GridComponent>(r.create(), grid);
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d