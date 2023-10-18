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

  if (s == Scene::game) {
    // create a cursor
    const auto cursor = create_gameplay(r, EntityType::cursor);

    // create a player
    auto e = create_gameplay(r, EntityType::actor_player);

    // create points on grid
    {
      GridComponent grid;
      grid.width = 100;
      grid.height = 100;
      // grid.size = 32;
      grid.grid.resize(grid.width * grid.height);
      // show a grid
      for (int xy = 0; xy < grid.width * grid.height; xy++) {
        //   const auto gpos = engine::grid::index_to_grid_position(xy, grid.width, grid.height);
        //   const auto sprite = create_gameplay(r, EntityType::empty);
        //   auto& spr_t = r.get<TransformComponent>(sprite);
        //   spr_t.position = { gpos.x * grid.size, gpos.y * grid.size, 0.0 };
        //   spr_t.scale = { 4, 4, 1 };
        // grid.grid[xy] = {};
      }
      // grid.flow_field = generate_flow_field(r, grid, hearth_idx);
      r.emplace<GridComponent>(r.create(), grid);
    }

    // create food/item dispencers
    for (int i = 0; i < 5; i++) {
      const auto e = create_gameplay(r, EntityType::actor_dispencer);
      r.get<AABB>(e).center = glm::ivec2{ 50, 0 } * i;
    }
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d