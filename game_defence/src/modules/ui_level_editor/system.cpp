#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/camera/helpers.hpp"
#include "renderer/components.hpp"

#include "glm/glm.hpp"
// #include "imfilebrowser.h"
#include "imgui.h"
#include "magic_enum.hpp"

#include <string>
#include <vector>

namespace game2d {

/* TODO:
What else do i need a level editor to do?
How to do scripts and events?
Serializing internal states?
e.g. a player has a backpack with 13 rocks in it
*/

void
update_ui_level_editor_system(entt::registry& r)
{
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);
  auto& level_editor = get_first_component<SINGLETON_LevelEditor>(r);

  ImGui::Begin("Level Editor");

  ImGui::Text("¬¬ Editor Mode ¬¬");
  {
    std::vector<std::string> modes;
    for (int i = 0; i < static_cast<int>(LevelEditorMode::count); i++) {
      LevelEditorMode value = magic_enum::enum_value<LevelEditorMode>(i);
      std::string value_str = std::string(magic_enum::enum_name(value));
      modes.push_back(value_str);
    }

    // which mode to run editor in?
    static int mode_current_idx = 0;
    {
      WomboComboIn in(modes);
      in.label = "select-mode";
      in.current_index = mode_current_idx;
      const auto out = draw_wombo_combo(in);
      if (out.selected != mode_current_idx) {
        const auto new_mode = magic_enum::enum_cast<LevelEditorMode>(modes[out.selected]).value();
        ;
        level_editor.mode = new_mode;

        // todo: if went from place to play,
        // keep a list of the entire state of the world during place

        // todo: if went from play to place,
        // do we have a list of the entire state of the previous world?
        // if so, go back to it
      }
      mode_current_idx = out.selected;
    }
    ImGui::Text("Running as %s", modes[mode_current_idx].c_str());
  }

  // TODO: load from disk
  // std::string path = "assets/maps/";
  // for (const auto& entry : std::filesystem::directory_iterator(path))
  //   maps.push_back(entry.path().generic_string());

  std::vector<std::string> levels{
    "assets/maps/map_0.json",
    "assets/maps/map_1.json",
    "assets/maps/map_2.json",
  };

  ImGui::Text("¬¬ Levels ¬¬");
  {
    static int selected = 0;
    for (int i = 0; i < levels.size(); i++) {
      const auto& level = levels[i];
      if (ImGui::Selectable(level.c_str(), selected == i))
        selected = i;
    }
    if (ImGui::Button("Refresh")) {
      // todo: refresh levels on disk
      selected = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
      if (levels.size() > 0) {
        const auto& level = levels[selected];
        // todo: load level
        // todo: are you sure prompt
      }
    }

    // TODO: only be able to save if you're in place-mode
    if (ImGui::Button("Save")) {
    }
  }

  ImGui::Text("¬¬ Prefabs ¬¬");
  {
    std::vector<std::string> items;
    for (int i = 0; i < static_cast<int>(EntityType::count); i++) {
      EntityType value = magic_enum::enum_value<EntityType>(i);
      std::string value_str = std::string(magic_enum::enum_name(value));
      items.push_back(value_str);
    }

    static int item_current_idx = 0;

    // which prefab?
    WomboComboIn in(items);
    in.label = "select-prefab";
    in.current_index = item_current_idx;
    const auto out = draw_wombo_combo(in);
    item_current_idx = out.selected;

    // overwrite size
    static bool overwrite_size = false;
    static int size = 16;
    ImGui::Checkbox("size##overwrite", &overwrite_size);
    ImGui::SameLine();
    int in_size = size;
    if (ImGui::DragInt("##overwritesize", &in_size))
      size = in_size;
    ImGui::Separator();

    // Entity To place!
    EntityType type = magic_enum::enum_cast<EntityType>(items[item_current_idx]).value();

    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto& input = get_first_component<SINGLETON_InputComponent>(r);
    const auto place_key = SDL_SCANCODE_T;
    const auto drag_key = SDL_SCANCODE_Y;
    const auto* place_key_name = SDL_GetScancodeName(place_key);
    const auto* drag_key_name = SDL_GetScancodeName(drag_key);
    ImGui::Text("Place Key: %s", place_key_name);
    ImGui::Text("Dragging Key: %s", drag_key_name);

    if (ri.viewport_hovered) {
      if (get_key_down(input, place_key)) {
        CreateEntityRequest request;
        request.type = type;
        request.position = { mouse_position.x, mouse_position.y, 0 };
        r.emplace<CreateEntityRequest>(r.create(), request);
      }
      if (get_key_held(input, drag_key)) {
        // dragging...
      }
    }
  }

  ImGui::End();
}

} // namespace game2d