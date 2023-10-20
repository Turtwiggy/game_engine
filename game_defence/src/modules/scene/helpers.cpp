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
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
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
      r.emplace<GridComponent>(r.create(), grid);
    }

    // create food/item dispencers
    for (int i = 0; i < 5; i++) {
      const auto e = create_gameplay(r, EntityType::actor_dispencer);
      r.get<AABB>(e).center = glm::ivec2{ (-250 + (50 * i)), 0 };
      r.get<AABB>(e).size = glm::ivec2{ 40, 40 };
      r.get<TransformComponent>(e).scale = glm::ivec3{ 40, 40, 1 };

      auto& pz = r.get<PickupZoneComponent>(e);
      pz.spawn_item_with_id = i;
    }

    // create delivery points
    {
      for (int i = 0; i < 2; i++) {
        const auto e = create_gameplay(r, EntityType::actor_customer_area);
        r.get<AABB>(e).center = glm::ivec2{ -250 + (250 * i), -250 };
        r.get<AABB>(e).size = glm::ivec2{ 50, 50 };
        r.get<TransformComponent>(e).scale = glm::ivec3{ 50, 50, 50 };
      }
    }
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d