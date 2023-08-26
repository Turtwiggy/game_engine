#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_bow/components.hpp"
#include "modules/combat/helpers.hpp"
#include "modules/gameover/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/ui_economy/components.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "ui_profiler/components.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s)
{
  {
    // entt::registry new_r;
    // transfer<SINGLETON_Animations>(r, new_r);
    // transfer<SINGLETON_Textures>(r, new_r);
    // transfer<SINGLETON_AudioComponent>(r, new_r);
    // transfer<SINGLETON_Profiler>(r, new_r);
    // transfer<SINGLETON_RendererInfo>(r, new_r);
    // transfer<SINGLETON_InputComponent>(r, new_r);
    // transfer<SINGLETON_FixedUpdateInputHistory>(r, new_r);
    // transfer<AudioSource>(r, new_r); // multiple of these
    // r = std::move(new_r);
  }

  for (const auto& [entity, comp] : r.view<TransformComponent>().each())
    r.destroy(entity); // visuals
  for (const auto& [entity, comp] : r.view<EntityTypeComponent>().each())
    r.destroy(entity); // actors
  for (const auto& [entity, comp] : r.view<CreateEntityRequest>().each())
    r.destroy(entity); // new actors

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_Economy>(r);
  destroy_and_create<SINGLETON_UiEconomy>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  destroy_and_create<SINGLETON_Wave>(r);
  const auto camera = create_gameplay(r, EntityType::camera);
  auto& camera_transform = r.get<TransformComponent>(camera);
  camera_transform.position = { 0, 0, 0 };
  camera_transform.scale.x = 0;
  camera_transform.scale.y = 0;

  // const auto cursor = create_gameplay(r, EntityType::cursor);

  if (s == Scene::game) {

    const auto hearth = create_gameplay(r, EntityType::actor_hearth);
    auto& hearth_transform = r.get<TransformComponent>(hearth);
    const glm::ivec2 spawn_pos = { 0, 0 };
    hearth_transform.position = { spawn_pos.x, spawn_pos.y, 0 };
    camera_transform.position = { spawn_pos.x, spawn_pos.y, 0 };

    const auto player = create_gameplay(r, EntityType::actor_player);
    auto& player_aabb = r.get<AABB>(player);
    player_aabb.center = { 32, 32 };

    // todo: generate random spawns
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d