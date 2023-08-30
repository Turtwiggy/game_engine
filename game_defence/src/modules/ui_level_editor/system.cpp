#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers.hpp"
#include "helpers/line.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "physics//components.hpp"
#include "renderer/components.hpp"

#include "glm/glm.hpp"
// #include "imfilebrowser.h"
#include "magic_enum.hpp"
#include <imgui.h>

#include <cstdio>
#include <string>
#include <vector>

namespace game2d {

/* TODO:
What else do i need a level editor to do?
- scripts and events?
- serializing hierarchical states?
- player has a backpack with 13 rocks in it, what do?
- click and drag object placements
*/

void
update_ui_level_editor_system(entt::registry& r, const glm::ivec2& input_mouse_pos)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& level_editor = get_first_component<SINGLETON_LevelEditor>(r);
  auto& lifecycle = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto place_key = SDL_SCANCODE_R;
  const auto place_and_drag_key = SDL_SCANCODE_T;
  const auto delete_key = SDL_SCANCODE_F;
  const auto snap_to_grid_key = SDL_SCANCODE_LSHIFT;
  // const auto select_entities_under_cursor_key = SDL_SCANCODE_E;
  const auto* place_key_name = SDL_GetScancodeName(place_key);
  const auto* place_and_drag_key_name = SDL_GetScancodeName(place_and_drag_key);
  const auto* delete_key_name = SDL_GetScancodeName(delete_key);
  const auto* snap_to_grid_key_name = SDL_GetScancodeName(snap_to_grid_key);

  glm::ivec2 mouse_pos = input_mouse_pos;
  if (get_key_held(input, snap_to_grid_key)) {
    const int grid_size = 5;
    const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, grid_size);
    mouse_pos = engine::grid::grid_space_to_world_space(grid_pos, grid_size);
  }

  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  if (scene.s == Scene::menu)
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::Begin("Level Editor", NULL, flags);
  ImGui::Text("Cursor: %i %i", mouse_pos.x, mouse_pos.y);
  ImGui::Text("Place Key: %s", place_key_name);
  ImGui::Text("Place/Drag Key: %s", place_and_drag_key_name);
  ImGui::Text("Delete Key: %s", delete_key_name);
  ImGui::Text("Snap To Grid Key: %s", snap_to_grid_key_name);

  ImGui::Text("¬¬ Editor Mode ¬¬");

  auto& mode = level_editor.mode;
  int mode_current_index = magic_enum::enum_index(mode).value();

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
    in.current_index = mode_current_index;
    const auto out = draw_wombo_combo(in);
    if (out.selected != mode_current_index) {
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
    mode_current_index = out.selected;
  }
  mode = magic_enum::enum_cast<LevelEditorMode>(modes[mode_current_index]).value();

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

    if (levels.size() > 0) {
      const std::string& level = levels[selected];

      //
      // save
      //
      if (mode == LevelEditorMode::edit) {
        // todo: are you sure
        if (ImGui::Button("Save"))
          save(r, level);
        ImGui::SameLine();
      } else
        ImGui::Text("Please go to edit mode to save level");

      //
      // load
      //
      if (ImGui::Button("Load")) {
        // todo: are you sure
        if (levels.size() > 0) {
          move_to_scene_start(r, scene.s);
          load(r, level);
        }
      }
      ImGui::SameLine();

      //
      // Delete
      //
      if (ImGui::Button("Delete")) {
        // TODO: are you sure
        move_to_scene_start(r, scene.s);

        std::remove(level.c_str());

        // refresh levels
        levels.clear();
        for (const auto& entry : std::filesystem::directory_iterator(path))
          levels.push_back(entry.path().generic_string());
        selected = 0;
      }

    } else
      ImGui::Text("No levels on disk!");

    //
    // Create a new level
    //
    if (ImGui::Button("Create")) {
      // move_to_scene_start(r, scene.s);

      std::string new_level_path = path + "level" + std::to_string(levels.size() - 1) + ".json";
      save(r, new_level_path);

      // refresh levels
      levels.clear();
      for (const auto& entry : std::filesystem::directory_iterator(path))
        levels.push_back(entry.path().generic_string());
      selected = levels.size() - 1;
    }

    //
    // Clear existing level
    //
    if (ImGui::Button("Clear")) {
      // todo: are you sure
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

    if (ri.viewport_hovered) {

      if (get_key_down(input, place_key)) {
        CreateEntityRequest request;
        request.type = type;
        request.transform = { { mouse_pos.x, mouse_pos.y, 0 } };
        r.emplace<CreateEntityRequest>(r.create(), request);
      }

      static entt::entity chosen_e;
      static glm::ivec2 initial_pos;

      if (get_key_down(input, place_and_drag_key)) {
        initial_pos = mouse_pos;
        chosen_e = create_gameplay(r, type);
      }
      if (get_key_held(input, place_and_drag_key)) {
        if (chosen_e != entt::null) {

          // set position for transform
          // auto& t = r.get<TransformComponent>(chosen_e);
          // set_line(r, t, initial_pos, mouse_pos);

          const auto line = generate_line(initial_pos, mouse_pos);

          // set position for aabb
          if (auto* aabb = r.try_get<AABB>(chosen_e))
            aabb->center = line.position;
          auto& transform = r.get<TransformComponent>(chosen_e);
          transform.scale = { line.scale.x, line.scale.y, 0 };
          transform.rotation_radians.z = line.rotation;
        }
      }
      if (get_key_up(input, place_and_drag_key))
        chosen_e = entt::null;

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

      //
    } // end hovered check
  }

  ImGui::End();
}

} // namespace game2d