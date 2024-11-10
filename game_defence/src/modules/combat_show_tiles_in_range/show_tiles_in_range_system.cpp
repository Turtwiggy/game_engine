#include "modules/combat_show_tiles_in_range/show_tiles_in_range_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"
#include "show_tiles_in_range_components.hpp"
#include "show_tiles_in_range_helpers.hpp"

#include <glm/fwd.hpp>
#include <imgui.h>

namespace game2d {

static bool show_distance_check = false;
static auto show_range_type = RangeType::knife;
static auto range_types_str = engine::enum_class_to_vec_str<RangeType>();

void
update_show_tiles_in_range_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

#if defined(_DEBUG)
  ImGui::Begin("ShowTile Mode");
  WomboComboIn combo_in(range_types_str);
  combo_in.label = "ShowTileMode";
  combo_in.current_index = static_cast<int>(show_range_type);
  WomboComboOut combo_out = draw_wombo_combo(combo_in);
  if (combo_in.current_index != combo_out.selected)
    show_range_type = static_cast<RangeType>(combo_out.selected);
  ImGui::End();
#endif

  const auto& view = r.view<const SelectedComponent, const InputComponent>();
  for (const auto& [e, selected_c, input_c] : view.each()) {
    auto& tiles_c = r.get_or_emplace<TilesComponent>(e);
    update_tiles_component(r, e, map_c, input_c, show_range_type, tiles_c);
  }

  //
}

} // namespace game2d