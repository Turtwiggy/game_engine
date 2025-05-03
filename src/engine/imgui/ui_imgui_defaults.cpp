#include "pch.hpp"

#include "ui_imgui_defaults.hpp"

namespace game2d {

void
imgui_begin(std::string label)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoBackground;

  ImGui::Begin(label.c_str(), NULL, flags);
}

} // namespace game2d