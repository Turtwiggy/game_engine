#include "ui_colours_system.hpp"

#include "modules/raws/raws_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_colours_system(entt::registry& r)
{
  const auto& raws = get_first_component<Raws>(r);

  ImGui::Begin("ui colours system");

  for (const auto& col : raws.colours) {
    // ImGui::Text("tag:%s hex:%s", col.tag.c_str(), col.hex.c_str());
    // ImGui::ColorEdit4(, float* col)
    const std::string label = col.tag;

    const auto srgb_col = hex_to_srgb(col.hex);
    float col_arr[4] = { srgb_col.r / 255.0f, srgb_col.g / 255.0f, srgb_col.b / 255.0f, srgb_col.a };
    ImGui::ColorEdit4(label.c_str(), col_arr);
  }

  ImGui::End();
};

} // namespace game2d