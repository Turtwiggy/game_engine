#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "physics//components.hpp"
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

Serializing hierarchical states?
e.g. a player has a backpack with 13 rocks in it

click and drag objects

*/

void
update_ui_level_editor_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  auto& level_editor = get_first_component<SINGLETON_LevelEditor>(r);
  auto& lifecycle = get_first_component<SINGLETON_EntityBinComponent>(r);

  ImGui::Begin("Level Editor");
  ImGui::Text("Cursor: %i %i", mouse_pos.x, mouse_pos.y);

  ImGui::Text("¬¬ Editor Mode ¬¬");

  static int mode_current_idx = 0;
  std::vector<std::string> modes;
  for (int i = 0; i < static_cast<int>(LevelEditorMode::count); i++) {
    LevelEditorMode value = magic_enum::enum_value<LevelEditorMode>(i);
    std::string value_str = std::string(magic_enum::enum_name(value));
    modes.push_back(value_str);
  }

  //
  // which mode to run editor in?
  //
  {
    WomboComboIn in(modes);
    in.label = "select-mode";
    in.current_index = mode_current_idx;
    const auto out = draw_wombo_combo(in);
    if (out.selected != mode_current_idx) {
      const auto new_mode = magic_enum::enum_cast<LevelEditorMode>(modes[out.selected]).value();

      // todo: if went from place to play,
      // keep a list of the entire state of the world during place
      if (new_mode == LevelEditorMode::play) {
        // ah!
      }

      // todo: if went from play to place,
      // do we have a list of the entire state of the previous world?
      // if so, go back to it
      if (new_mode == LevelEditorMode::edit) {
        // oh!
      }

      level_editor.mode = new_mode;
    }
    mode_current_idx = out.selected;
  }

  const LevelEditorMode mode = magic_enum::enum_cast<LevelEditorMode>(modes[mode_current_idx]).value();

  //
  // scan disk for levels
  //
  std::string path = "assets/maps/";
  std::vector<std::string> levels;
  for (const auto& entry : std::filesystem::directory_iterator(path))
    levels.push_back(entry.path().generic_string());

  ImGui::Text("¬¬ Levels ¬¬");
  {
    static int selected = 0;
    for (int i = 0; i < levels.size(); i++) {
      const auto& level = levels[i];
      if (ImGui::Selectable(level.c_str(), selected == i))
        selected = i;
    }
    if (levels.size() == 0)
      ImGui::Text("No levels on disk!");

    if (ImGui::Button("Create")) {
      // todo: create new level
    }

    // only be able to save if you're in edit-mode
    if (mode == LevelEditorMode::edit) {
      ImGui::SameLine();
      if (ImGui::Button("Save"))
        save(r, "assets/maps/temp.json");
      ImGui::SameLine();
    } else
      ImGui::Text("Please go to edit mode to save level");

    if (ImGui::Button("Load")) {
      if (levels.size() > 0) {
        const auto& level = levels[selected];
        // todo: use that level string
        load(r, "assets/maps/temp.json");
      }
    }
    ImGui::SameLine();

    if (ImGui::Button("Clear")) {
      // todo: are you sure

      const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
      move_to_scene_start(r, scene.s);
    }

    if (ImGui::Button("Refresh")) {
      levels.clear();
      for (const auto& entry : std::filesystem::directory_iterator(path))
        levels.push_back(entry.path().generic_string());
      selected = 0;
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
    const auto place_key = SDL_SCANCODE_R;
    const auto drag_key = SDL_SCANCODE_T;
    const auto delete_key = SDL_SCANCODE_F;
    const auto select_entities_under_cursor_key = SDL_SCANCODE_E;
    const auto* place_key_name = SDL_GetScancodeName(place_key);
    const auto* drag_key_name = SDL_GetScancodeName(drag_key);
    const auto* delete_key_name = SDL_GetScancodeName(delete_key);
    ImGui::Text("Place Key: %s", place_key_name);
    ImGui::Text("Dragging Key: %s", drag_key_name);
    ImGui::Text("Delete Key: %s", delete_key_name);

    if (ri.viewport_hovered) {
      if (get_key_down(input, place_key)) {
        CreateEntityRequest request;
        request.type = type;
        request.position = { mouse_pos.x, mouse_pos.y, 0 };
        r.emplace<CreateEntityRequest>(r.create(), request);
      }
      if (get_key_held(input, drag_key)) {
        // todo: dragging...
      }

      // delete anything that collides with the cursor

      if (get_key_held(input, delete_key)) {
        ImGui::Text("deleting objects...");
        const auto cursor_entity = get_first<CursorComponent>(r);
        for (const auto& coll : physics.collision_stay) {
          if (coll.ent_id_0 == static_cast<uint32_t>(cursor_entity))
            lifecycle.dead.emplace(static_cast<entt::entity>(coll.ent_id_1));
          else if (coll.ent_id_1 == static_cast<uint32_t>(cursor_entity))
            lifecycle.dead.emplace(static_cast<entt::entity>(coll.ent_id_0));
        }
      }
    } // end hovered check
  }

  ImGui::End();
}

} // namespace game2d