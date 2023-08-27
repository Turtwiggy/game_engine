#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_bow/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameover/components.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s)
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

  const auto& camera = get_first<OrthographicCamera>(r);
  auto& camera_transform = get_first_component<TransformComponent>(r);

  for (const auto& [entity, _] : r.view<TransformComponent>().each()) {
    if (entity == camera)
      continue;
    r.destroy(entity);
  }
  for (const auto& [entity, _] : r.view<EntityTypeComponent>().each()) {
    if (entity == camera)
      continue;
    r.destroy(entity); // actors
  }
  const auto& reqs = r.view<CreateEntityRequest>();
  r.destroy(reqs.begin(), reqs.end());

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  destroy_and_create<SINGLETON_Wave>(r);

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