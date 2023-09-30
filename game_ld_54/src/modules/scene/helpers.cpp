#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"

#include "modules/actor_bow/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"

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
  if (level == 0) {
    GridComponent grid;
    grid.width = 10;
    grid.height = 10;
    grid.size = 16;
    grid.grid.resize(grid.width * grid.height);

    // create a spawner
    {
      const glm::ivec2 grid_pos{ 0, 0 };
      const auto grid_idx = engine::grid::grid_position_to_index(grid_pos, grid.width);
      const auto pos = engine::grid::grid_space_to_world_space(grid_pos, grid.size);

      const auto e = create_gameplay(r, EntityType::actor_spawner);

      auto& t = r.get<AABB>(e);
      t.center = pos;

      grid.grid[grid_idx].push_back(e);
    }

    // create a hearth
    {
      const glm::ivec2 grid_pos{ 9, 9 };
      const auto grid_idx = engine::grid::grid_position_to_index(grid_pos, grid.width);
      const auto pos = engine::grid::grid_space_to_world_space(grid_pos, grid.size);

      const auto e = create_gameplay(r, EntityType::actor_hearth);
      auto& t = r.get<AABB>(e);
      t.center = pos;

      grid.grid[grid_idx].push_back(e);
    }

    // create some grid vfx
    for (int xy = 0; xy < grid.width * grid.height; xy++) {
      const auto gpos = engine::grid::index_to_grid_position(xy, grid.width, grid.height);
      const auto empty = create_gameplay(r, EntityType::empty);
      auto& t = r.get<TransformComponent>(empty);

      // offset center to tl;
      // const glm::ivec3 offset = { -grid.size / 2, -grid.size / 2, 0 };
      const glm::ivec3 offset{ 0, 0, 0 };
      t.position = { gpos.x * grid.size, gpos.y * grid.size, 0.0 };
      t.position += offset;
      t.scale = { 1, 1, 1 };
    }

    r.emplace<GridComponent>(r.create(), grid);
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d