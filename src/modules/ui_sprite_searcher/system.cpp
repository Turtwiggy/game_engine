#include "system.hpp"

#include "modules/sprites/helpers.hpp"
#include "modules/ui_sprite_searcher/components.hpp"
#include "resources/textures.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <stdio.h>

void
game2d::update_ui_sprite_searcher_system(entt::registry& r)
{
  // #ifdef _DEBUG
  // bool show_imgui_demo_window = false;
  // ImGui::ShowDemoWindow(&show_imgui_demo_window);
  // #endif

  const auto& slots = r.ctx().at<SINGLETON_Textures>();
  auto& ss = r.ctx().at<SINGLETON_SpriteSearcher>();

  // texture information
  const float px = 768.0;
  const float py = 352.0;
  const int wx = 48;
  const int wy = 22;
  const int pixels = 16;

  // display size
  const float sizex = 16.0f;
  const float sizey = 16.0f;

  // searchbar input
  static std::string value = "";

  ImGui::Begin("Sprite Searcher");
  std::string label = "Search: ";
  ImGui::Text(label.c_str());
  ImGui::SameLine();
  ImGui::InputText((std::string("##") + label).c_str(), &value);

  // ImVec2 scrolling_child_size = ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 10);
  ImVec2 scrolling_child_size = ImVec2(0, 0);
  ImGui::BeginChild("scrolling", scrolling_child_size, true, ImGuiWindowFlags_HorizontalScrollbar);

  for (int y = 0; y < wy; y++) {
    for (int x = 0; x < wx; x++) {

      // these are for the full texture
      // ImVec2 tl = ImVec2(0.0f, 0.0f);
      // ImVec2 br = ImVec2(1.0f, 1.0f);

      const float tlx = static_cast<float>(x * pixels) / px;
      const float tly = static_cast<float>(y * pixels) / py;
      const float blx = static_cast<float>(x * pixels + pixels) / px;
      const float bly = static_cast<float>(y * pixels + pixels) / py;
      ImVec2 tl = ImVec2(tlx, tly);
      ImVec2 br = ImVec2(blx, bly);

      char buffer[64];
      sprintf(buffer, "x%iy%i", x, y);
      if (ImGui::ImageButton(buffer, (ImTextureID)slots.tex_id_kenny, { sizex, sizey }, tl, br)) {
        ss.x = x;
        ss.y = y;
      }
      ImGui::SameLine();
    }
    ImGui::NewLine();
  }

  // float scroll_x = ImGui::GetScrollX();
  // float scroll_max_x = ImGui::GetScrollMaxX();
  ImGui::EndChild();

  ImGui::End();
};