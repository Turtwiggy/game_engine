// your header
#include "modules/map_editor/system.hpp"

// components
#include "components/singleton_grid.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_map_editor/components.hpp"

// helpers
#include "helpers/physics_layers.hpp"

// other engine headers
#include "engine/grid.hpp"

void
game2d::init_map_editor_system(entt::registry& registry){
  //
};

void
game2d::update_map_editor_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();
  // dont process game events if the viewport says so
  if (!ri.viewport_process_events)
    return;

  const auto& gs = registry.ctx<SINGLETON_GridSize>();
  const int GRID_SIZE = gs.size_xy;

  // urgh.. x3
  glm::ivec2 imgui_mouse_pos = app.get_input().get_mouse_pos();
  glm::vec2 imgui_viewport_tl = ri.viewport_pos;
  glm::ivec2 mouse_pos = imgui_mouse_pos - glm::ivec2(imgui_viewport_tl.x, imgui_viewport_tl.y);
  // adjust pos to keep square visible while mouse is in 4 corners
  glm::vec2 mouse_pos_adjusted_in_worldspace = mouse_pos;
  mouse_pos_adjusted_in_worldspace.x += GRID_SIZE / 2.0f;
  mouse_pos_adjusted_in_worldspace.y += GRID_SIZE / 2.0f;

  const auto& map_editor_info = registry.ctx<SINGLETON_MapEditorInfo>();
  if (map_editor_info.place_sprite && app.get_input().get_mouse_lmb_down()) {
    glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(mouse_pos_adjusted_in_worldspace, GRID_SIZE);
    glm::ivec2 world_space = grid_slot * GRID_SIZE;

    // The entity to spawn!
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "entity");
    registry.emplace<ColourComponent>(r, 1.0f, 1.0f, 1.0f, 1.0f);
    registry.emplace<PositionIntComponent>(r, world_space.x, world_space.y);
    registry.emplace<RenderSizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<PhysicsSizeComponent>(r, GRID_SIZE, GRID_SIZE);
    const auto& sprite = map_editor_info.sprite_to_place.value_or(sprite::type::EMPTY);
    registry.emplace<SpriteComponent>(r, sprite);
    registry.emplace<VelocityComponent>(r);
    registry.emplace<CollidableComponent>(r, static_cast<uint32_t>(GameCollisionLayer::SOLID_WALL), PhysicsType::SOLID);
  }
}
