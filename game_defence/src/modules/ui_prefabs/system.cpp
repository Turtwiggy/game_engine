#include "system.hpp"
#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/camera/helpers.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "sprites/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

#include <map>

namespace game2d {

void
update_ui_prefabs_system(entt::registry& r)
{
  auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  // auto& ss = get_first_component<SINGLETON_SpriteSearcher>(r);

  const int GRID_SIZE = 16; // hmm
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r) + glm::ivec2(GRID_SIZE / 2, GRID_SIZE / 2);
  const glm::ivec2 grid_position = engine::grid::world_space_to_grid_space(mouse_position, GRID_SIZE);
  const glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_position, GRID_SIZE);

  // this seems weird, but it's because std::pair is serializable
  // by the std::map type, whereas the glm::ivec2 isn't by default
  std::pair<int, int> key = { world_position.x, world_position.y };

  // TODO: be able to update sprite colour
  // TODO: be able to select sprite game behaviour
  // TODO: save the configs above as prefabs
  ImGui::Begin("Prefabs");

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
  // Tilemap GameEditor
  //
  {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    if (!ri.viewport_hovered)
      return;

    bool place_mode = get_mouse_mmb_press();
    bool delete_mode = false;
    // bool empty_space = !tilemap.tilemap.contains(key);

    // Note: this creation should be deferred to FixedUpdate if
    // this is ever used in gameplay reasons (other than mapping tools)

    if (place_mode) {
      CreateEntityRequest request;
      request.type = type;
      request.position = { world_position.x, world_position.y, 0 };
      r.emplace<CreateEntityRequest>(r.create(), request);
    }
  }
};

} // namespace game2d