#include "modules/ui_arrows_to_spawners/system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ui_arrows_to_spawners/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
create_ui_arrow(entt::registry& r, SINGLE_ArrowsToSpawnerUI& ui, const int i)
{
  const auto e = create_gameplay(r, EntityType::empty_with_transform);
  set_sprite(r, e, "ARROW_UP");
  ui.instantiated.push_back(e);
};

void
destroy_ui_arrow(entt::registry& r, SINGLE_ArrowsToSpawnerUI& ui, const int i)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  const auto idx = i - 1;
  auto entity = ui.instantiated[idx];
  dead.dead.emplace(entity);
  ui.instantiated.erase(ui.instantiated.begin() + idx);
};

void
update_ui_arrow(entt::registry& r,
                const SINGLETON_RendererInfo& ri,
                const SINGLE_ArrowsToSpawnerUI& ui,
                const int i,
                const glm::vec2& spawner_position_worldspace)
{
  const auto viewport_size = ri.viewport_size_render_at;
  const float viewport_w_half = viewport_size.x / 2.0f;
  const float viewport_h_half = viewport_size.y / 2.0f;
  const auto camera = get_first<OrthographicCamera>(r);
  const auto& camera_transform = r.get<TransformComponent>(camera);

  // get spawner position relative to camera
  const glm::vec2 camera_pos = glm::vec2(camera_transform.position.x, camera_transform.position.y);
  const glm::vec2 pos_relative_to_camera = spawner_position_worldspace - camera_pos;

  // Finally, update the arrow position
  const int offset_from_screen_edge = 50;
  const auto arrow = ui.instantiated[i];
  auto& arrow_transform = r.get<TransformComponent>(arrow);
  const auto max_x_offset = glm::clamp(
    pos_relative_to_camera.x, -viewport_w_half + offset_from_screen_edge, viewport_w_half - offset_from_screen_edge);
  const auto max_y_offset = glm::clamp(
    pos_relative_to_camera.y, -viewport_h_half + offset_from_screen_edge, viewport_h_half - offset_from_screen_edge);
  arrow_transform.position.x = camera_transform.position.x + max_x_offset;
  arrow_transform.position.y = camera_transform.position.y + max_y_offset;
}

void
update_ui_arrows_to_spawners_system(entt::registry& r)
{
  auto& ui = get_first_component<SINGLE_ArrowsToSpawnerUI>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& view = r.view<const SpawnerComponent, const OnlySpawnInRangeOfAnyPlayerComponent, const AABB>();
  const int cur_size = view.size_hint();
  const int old_size = ui.instantiated.size();

  // create arrows
  const int to_create = cur_size - old_size;
  for (int i = 0; i < to_create; i++)
    create_ui_arrow(r, ui, i);

  // destroy arrows
  for (auto i = old_size; i > cur_size; i--)
    destroy_ui_arrow(r, ui, i);

  // update arrows to positioons relative to spawner positions
  for (int i = 0; const auto& [e, spawner, spawner_limit, aabb] : view.each()) {
    update_ui_arrow(r, ri, ui, i, glm::vec2(aabb.center));
    i++;
  }
}

} // namespace game2d