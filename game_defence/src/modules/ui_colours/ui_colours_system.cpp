#include "ui_colours_system.hpp"

#include "engine/colour/colour.hpp"
#include "modules/raws/raws_components.hpp"

#include "imgui.h"

namespace game2d {

engine::SRGBColour
hex_to_srgb(const std::string& hex)
{
  const std::string hex_without_hash = hex.substr(hex.find("#") + 1, hex.length());
  const std::string str_r = hex_without_hash.substr(0, 2);
  const std::string str_g = hex_without_hash.substr(2, 2);
  const std::string str_b = hex_without_hash.substr(4, 2);
  const int r = std::stoi(str_r, 0, 16);
  const int g = std::stoi(str_g, 0, 16);
  const int b = std::stoi(str_b, 0, 16);
  return { r, g, b, 1.0 };
};

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