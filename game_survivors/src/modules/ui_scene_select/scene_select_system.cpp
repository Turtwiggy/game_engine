#include "scene_select_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/system_hulls/hulls_components.hpp"
#include "scene_select_components.hpp"

#include <imgui.h>

#include <format>

namespace game2d {

// Display all the hulls,
// and the player can select one
void
update_ui_scene_select_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto x_pos = (6 / 12.0f);
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + (ri.viewport_size_render_at.x * x_pos), viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  const ImVec2 size = { 120.0f, 40.0f };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

  ImGui::Begin("Hull", NULL, flags);

  ImGui::SeparatorText("Select a hull");
  const auto text_col = ImVec4(0.64f, 0.64f, 0.64f, 1.0f);

  const auto align_right_button = [](std::string label) -> bool {
    // Align the button to the right
    const float available_width = ImGui::GetContentRegionAvail().x;
    const float button_width = 60.0f; // Set the desired button width
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available_width - button_width);
    return ImGui::Button(label.c_str(), ImVec2(button_width, 0));
  };

  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);

  auto sorted_hulls = hulls_c.hulls;
  auto sort_by_hullsize = [](const ShipHullData& a, const ShipHullData& b) {
    int size_a = a.height * a.width;
    int size_b = b.height * b.height;
    return size_a < size_b;
  };
  std::sort(sorted_hulls.begin(), sorted_hulls.end(), sort_by_hullsize);

  for (const auto& hull_data : sorted_hulls) {

    // Name
    ImGui::Text("%s", hull_data.name.c_str());

    // Description
    ImGui::SameLine();
    ImGui::TextColored(text_col, "%s", hull_data.desc.c_str());

    // Select
    auto label = std::format("Select##{}", hull_data.name);
    ImGui::SameLine();
    if (ImGui::Button(label.c_str())) {

      // create data with selected hull
      SelectSceneToSurviveScene data;
      data.chosen_boat = hull_data.name;
      create_persistent<SelectSceneToSurviveScene>(r, data);
      SDL_Log("Launching survive with hull: %s", data.chosen_boat.c_str());

      move_to_scene_start(r, Scene::survive);
    }
  }

  ImGui::End();
  ImGui::PopStyleVar(3);
}

} // namespace game2d