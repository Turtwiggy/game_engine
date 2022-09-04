#include "system.hpp"
#include "components.hpp"

#include "engine/maths/grid.hpp"
#include "game/entities/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/ui_sprite_searcher/components.hpp"

#include <imgui.h>

#include <map>

static int GRID_SIZE = 16; // hmm

void
game2d::update_ui_editor_tilemap_system(entt::registry& r)
{
  const auto& input = r.ctx().at<SINGLETON_InputComponent>();
  auto& ss = r.ctx().at<SINGLETON_SpriteSearcher>();
  auto& tilemap = r.ctx().at<SINGLETON_TilemapComponent>();

  ImGui::Text("Prefabs");
  ImGui::Text("TODO");
  // TODO: show selected sprite
  // TODO: be able to update sprite colour
  // TODO: be able to select sprite game behaviour
  // TODO: save the configs above as prefabs
  // ImGui::Text("SS: %s", ss.clicked.c_str());
  ImGui::Separator();

  glm::ivec2 mouse_grid_pos =
    engine::grid::world_space_to_clamped_world_space(input.mouse_position_in_worldspace, GRID_SIZE);
  std::pair<int, int> pos = { mouse_grid_pos.x, mouse_grid_pos.y };

  const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
  if (!ri.viewport_process_events)
    return; // dont place sprites if selecting ui

  // Note: this creation should be deffered to FixedUpdate if
  // this is ever used in gameplay reasons (other than mapping tools)
  if (get_mouse_rmb_held()) {

    // check tilemap is empty
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
    } else {
      //
      // overwrite space!
      //
      auto& e = tilemap.tilemap[pos];
      auto& sprite = r.get<SpriteComponent>(e);
      sprite.x = ss.x;
      sprite.y = ss.y;
    }
  }

  if (get_mouse_mmb_held()) {
    if (tilemap.tilemap.contains(pos)) {
      r.destroy(tilemap.tilemap[pos]); // remove entity
      tilemap.tilemap.erase(pos);      // remove from tilemap
    }
  }
};