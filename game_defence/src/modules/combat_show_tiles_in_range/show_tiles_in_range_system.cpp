#include "modules/combat_show_tiles_in_range/show_tiles_in_range_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "modules/actor_door/door_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"
#include "show_tiles_in_range_components.hpp"
#include "show_tiles_in_range_helpers.hpp"

#include <glm/fwd.hpp>
#include <imgui.h>

namespace game2d {

void
update_show_tiles_in_range_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(MapComponent, r, map_e, map_c);

  // static bool show_distance_check = false;
  // static auto show_range_type = RangeType::knife;
  // static auto range_types_str = engine::enum_class_to_vec_str<RangeType>();
  // #if defined(_DEBUG)
  //   ImGui::Begin("ShowTile Mode");
  //   WomboComboIn combo_in(range_types_str);
  //   combo_in.label = "ShowTileMode";
  //   combo_in.current_index = static_cast<int>(show_range_type);
  //   WomboComboOut combo_out = draw_wombo_combo(combo_in);
  //   if (combo_in.current_index != combo_out.selected)
  //     show_range_type = static_cast<RangeType>(combo_out.selected);
  //   ImGui::End();
  // #endif

  const auto& view = r.view<const SelectedComponent, const InputComponent>();
  for (const auto& [e, selected_c, input_c] : view.each()) {
    auto& tiles_c = r.get_or_emplace<TilesComponent>(e);
    auto item_e = get_equipped_gun(r, e);

    // default range type if no weapon
    if (item_e == entt::null) {
      tiles_c.tiles.clear();
      continue;
    }

    std::vector<glm::ivec2> tiles;

    const auto gp = get_grid_position(r, e);
    const auto& item_c = r.get<Item>(item_e);

    if (!item_c.combat.has_value()) {
      // SDL_Log("Item does not have combat attribute");
      tiles_c.tiles.clear();
      continue;
    }

    const auto input = glm::ivec2{ input_c.rx, input_c.ry };
    const auto& type = item_c.combat->type;
    const auto range = item_c.combat->range;

    if (type == "cone")
      tiles = get_tiles_for_shotgun(r, map_c, gp, input, range);

    if (type == "line")
      tiles = get_tiles_in_line(r, map_c, gp, input, range);

    if (type == "area")
      tiles = get_tiles_in_area(r, map_c, gp, range);

    if (type == "other_team_actor_in_range")
      tiles = get_tiles_in_area(r, map_c, gp, range);

    // else
    //   SDL_Log("Unknown tile type: %s", type.c_str());

    tiles_c.tiles = tiles;
  }
}

} // namespace game2d