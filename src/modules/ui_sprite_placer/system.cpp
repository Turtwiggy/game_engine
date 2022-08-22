#include "system.hpp"

#include "engine/maths/grid.hpp"
#include "game/entities/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/ui_sprite_placer/components.hpp"
#include "modules/ui_sprite_searcher/components.hpp"

#include <map>

#include <imgui.h>

void
game2d::update_ui_sprite_placer_system(entt::registry& r)
{
  const auto& input = r.ctx().at<SINGLETON_InputComponent>();
  auto& ss = r.ctx().at<SINGLETON_SpriteSearcher>();
  auto& tilemap = r.ctx().at<SINGLETON_TilemapComponent>();

  // ImGui::Begin("Sprite Placer");
  // ImGui::Text("SS: %s", ss.clicked.c_str());
  // ImGui::End();

  // Note: this creation should be deffered to FixedUpdate if
  // this is ever used in gameplay reasons (other than mapping tools)
  if (get_mouse_rmb_held()) {

    const int GRID_SIZE = 16;
    glm::ivec2 mouse_grid_pos =
      engine::grid::world_space_to_clamped_world_space(input.mouse_position_in_worldspace, GRID_SIZE);

    // check tilemap is empty
    std::pair<int, int> pos = { mouse_grid_pos.x, mouse_grid_pos.y };
    if (!tilemap.tilemap.contains(pos)) {
      //
      // empty space!
      //
      auto e = create_entity(r, ENTITY_TYPE::POTION);
      create_renderable(r, e, ENTITY_TYPE::POTION);

      auto& transform = r.get<TransformComponent>(e);
      // transform.position.x = input.mouse_position_in_worldspace.x;
      // transform.position.y = input.mouse_position_in_worldspace.y;
      transform.position.x = mouse_grid_pos.x;
      transform.position.y = mouse_grid_pos.y;

      auto& sprite = r.get<SpriteComponent>(e);
      sprite.x = ss.x;
      sprite.y = ss.y;

      tilemap.tilemap[pos] = e;
    }
  }
};