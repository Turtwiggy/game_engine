#include "system.hpp"
#include "components.hpp"

#include "engine/colour.hpp"
#include "engine/maths/grid.hpp"
#include "game/entities/actors.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_sprite_searcher/components.hpp"
#include "resources/colour.hpp"

#include "magic_enum.hpp"
#include <imgui.h>

#include <map>

void
game2d::update_ui_editor_tilemap_system(entt::registry& r)
{
  // #ifdef _DEBUG
  //   bool show_imgui_demo_window = true;
  //   ImGui::ShowDemoWindow(&show_imgui_demo_window);
  // #endif

  //
  // Prefab Editor
  //

  auto& ss = r.ctx().at<SINGLETON_SpriteSearcher>();
  auto& tilemap = get_first<TilemapComponent>(r);
  auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();
  const int GRID_SIZE = 16; // hmm
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r) + glm::ivec2(GRID_SIZE / 2, GRID_SIZE / 2);
  const glm::ivec2 grid_position = engine::grid::world_space_to_clamped_world_space(mouse_position, GRID_SIZE);

  // this seems weird, but it's because std::pair is serializable
  // by the std::map type, whereas the glm::ivec2 isn't by default
  std::pair<int, int> key = { grid_position.x, grid_position.y };

  // TODO: be able to update sprite colour
  // TODO: be able to select sprite game behaviour
  // TODO: save the configs above as prefabs
  ImGui::Begin("Prefabs innit");

  static bool overwrite_sprite = false;
  ImGui::Checkbox("sprite##overwrite", &overwrite_sprite);
  ImGui::SameLine();
  ImGui::Text("With: %i %i", ss.x, ss.y);
  ImGui::Separator();

  static bool overwrite_angle = false;
  static float angle = 0.0f;
  ImGui::Checkbox("angle", &overwrite_angle);
  ImGui::SameLine();
  imgui_draw_float("angle", angle);
  ImGui::Separator();

  static bool overwrite_colour = false;
  static engine::SRGBColour rgba = colours.white;
  ImGui::Checkbox("colour##overwrite", &overwrite_colour);
  ImGui::SameLine();
  static float rgba_cols[4] = { rgba.r, rgba.g, rgba.b, rgba.a };
  if (ImGui::ColorEdit4("##overwritecolouredit", rgba_cols))
    rgba = { rgba_cols[0], rgba_cols[1], rgba_cols[2], rgba_cols[3] };
  ImGui::Separator();

  static bool overwrite_size = false;
  static int size = 16;
  ImGui::Checkbox("size##overwrite", &overwrite_size);
  ImGui::SameLine();
  int in_size = size;
  if (ImGui::DragInt("##overwritesize", &in_size))
    size = in_size;
  ImGui::Separator();

  std::vector<std::string> items;
  for (int i = 0; i < static_cast<int>(EntityType::count); i++) {
    EntityType value = magic_enum::enum_value<EntityType>(i);
    std::string value_str = std::string(magic_enum::enum_name(value));
    items.push_back(value_str);
  }
  ImGui::Text("Size: %i", items.size());
  ImGui::Separator();

  static ImGuiComboFlags flags = 0;
  static int item_current_idx = 0; // Here we store our selection data as an index.

  // Pass in the preview value visible before opening the combo (it could be anything)
  const char* combo_preview_value = items[item_current_idx].c_str();
  if (ImGui::BeginCombo("wombocombo", combo_preview_value, flags)) {
    for (int n = 0; n < items.size(); n++) {
      const bool is_selected = (item_current_idx == n);
      if (ImGui::Selectable(items[n].c_str(), is_selected))
        item_current_idx = n;
      // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  ImGui::End();

  // Entity To place!
  EntityType type = magic_enum::enum_value<EntityType>(item_current_idx);

  //
  // Tilemap Editor
  //
  {
    const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
    if (!ri.viewport_process_events)
      return; // dont place sprites if selecting ui

    bool place_mode = get_mouse_rmb_held();
    bool delete_mode = get_mouse_mmb_held();
    bool empty_space = !tilemap.tilemap.contains(key);

    // Note: this creation should be deferred to FixedUpdate if
    // this is ever used in gameplay reasons (other than mapping tools)

    if (place_mode) {

      entt::entity e;

      if (empty_space) {
        // create
        e = create_gameplay(r, type);
        create_renderable(r, e, type);
        auto& transform = r.get<TransformComponent>(e);
        transform.position.x = grid_position.x;
        transform.position.y = grid_position.y;
        tilemap.tilemap[key] = e;
      } else {
        // update
        e = tilemap.tilemap[key];
      }

      if (overwrite_sprite) {
        auto& sprite = r.get<SpriteComponent>(e);
        sprite.x = ss.x;
        sprite.y = ss.y;
      }

      if (overwrite_angle) {
        auto& sprite = r.get<SpriteComponent>(e);
        sprite.angle_radians = glm::radians(angle);
      }

      if (overwrite_colour) {
        auto& sprite = r.get<SpriteColourComponent>(e);
        sprite.colour = engine::SRGBToLinear(rgba);
      }

      if (overwrite_size) {
        auto& sprite_t = r.get<TransformComponent>(e);
        sprite_t.scale.x = size;
        sprite_t.scale.y = size;
        auto sprite_po = r.try_get<PhysicsSizeComponent>(e);
        if (sprite_po) {
          sprite_po->w = size;
          sprite_po->h = size;
        }
      }
    }

    if (delete_mode && !empty_space) {
      r.destroy(tilemap.tilemap[key]); // remove entity
      tilemap.tilemap.erase(key);      // remove from tilemap
    }
  }
};