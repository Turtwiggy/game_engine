#include "system.hpp"

#include <imgui.h>

#include <string>

void
game2d::update_ui_colours_system(SINGLETON_ColoursComponent& colours)
{

  ImGui::Begin("Colour Editor");
  auto srgb_primary = colours.primary;
  auto srgb_secondary = colours.secondary;
  auto srgb_tertiary = colours.tertiary;
  auto srgb_quaternary = colours.quaternary;

  const auto colour_widget = [](const std::string& tag, engine::SRGBColour& col, engine::LinearColour& lin_col) {
    float cols[4] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a };
    if (ImGui::ColorEdit4(tag.c_str(), cols)) {

      engine::SRGBColour result;
      result.r = static_cast<int>(cols[0] * 255.0f);
      result.g = static_cast<int>(cols[1] * 255.0f);
      result.b = static_cast<int>(cols[2] * 255.0f);
      result.a = cols[3];

      col = result;
      lin_col = engine::SRGBToLinear(col);
    }
  };
  colour_widget("primary##primary", colours.primary, colours.lin_primary);
  colour_widget("secondary##secondary", colours.secondary, colours.lin_secondary);
  colour_widget("tertiary##tertiary", colours.tertiary, colours.lin_tertiary);
  colour_widget("quaternary##quaternary", colours.quaternary, colours.lin_quaternary);

  ImGui::End();
};