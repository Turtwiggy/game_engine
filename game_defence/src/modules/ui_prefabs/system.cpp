#include "system.hpp"
#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/mouse.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/actor_spawner/components.hpp"
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
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  // auto& ss = get_first_component<SINGLETON_SpriteSearcher>(r);

  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);

  // this seems weird, but it's because std::pair is serializable
  // by the std::map type, whereas the glm::ivec2 isn't by default

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

  static int item_current_idx = 0;

  // all of the enums for EntityType
  std::vector<std::string> items;
  for (int i = 0; i < static_cast<int>(EntityType::count); i++) {
    EntityType value = magic_enum::enum_value<EntityType>(i);
    std::string value_str = std::string(magic_enum::enum_name(value));
    items.push_back(value_str);
  }

  WomboComboIn in(items);
  in.label = "label";
  in.current_index = item_current_idx;
  const auto out = draw_wombo_combo(in);
  item_current_idx = out.selected;

  // Entity To place!
  EntityType type = magic_enum::enum_cast<EntityType>(items[item_current_idx]).value();

  //
  // Choose spawner type
  //
  if (type == EntityType::actor_spawner) {
    static int spawner_item_current_idx = 0;
    WomboComboIn in(items);
    in.label = "spawner-items";
    in.current_index = spawner_item_current_idx;
    const auto out = draw_wombo_combo(in);
    spawner_item_current_idx = out.selected;

    const auto& spawned_items = get_first_component<SINGLETON_EntityBinComponent>(r);
    if (spawned_items.created_this_frame.size() > 0) {
      for (int i = 0; i < spawned_items.created_this_frame.size(); i++) {
        // check if we spawned a spawner
        const auto& e_item = spawned_items.created_this_frame[i];
        const auto& e_type = r.get<EntityTypeComponent>(e_item);
        if (e_type.type == EntityType::actor_spawner) {
          auto& e_spawner = r.get<SpawnerComponent>(e_item);

          e_spawner.type_to_spawn = magic_enum::enum_value<EntityType>(spawner_item_current_idx);
        }
      }
    }
  }

  ImGui::End();

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
      request.position = { mouse_position.x, mouse_position.y, 0 };
      r.emplace<CreateEntityRequest>(r.create(), request);
    }
  }
};

} // namespace game2d