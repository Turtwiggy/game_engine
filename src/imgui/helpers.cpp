#include "helpers.hpp"

#include "imgui.h"

namespace game2d {

WomboComboOut
draw_wombo_combo(const WomboComboIn& in)
{
  const auto& label = in.label;
  const auto& items = in.items;
  const auto& cur_idx = in.current_index;

  WomboComboOut out;
  out.selected = in.current_index;

  const char* combo_preview_value = items[in.current_index].c_str();

  if (ImGui::BeginCombo(label.c_str(), combo_preview_value, 0)) {
    for (int n = 0; n < items.size(); n++) {
      const bool is_selected = (cur_idx == n);

      if (ImGui::Selectable(items[n].c_str(), is_selected))
        out.selected = n;

      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  return out;
};

} // namespace game2d