#include "pch.hpp"

#include "ui_debug_menubar_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"

namespace game2d {

void
update_ui_debug_menubar_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_DebugMenuBar, r, menu_e, menu_c);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_MenuBar; // dont copy paste this
  // flags |= ImGuiWindowFlags_NoSavedSettings;
  // flags |= ImGuiWindowFlags_NoInputs;

  auto size = ImVec2{ (float)160, 20 };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always);
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);

  bool p_open = true;
  ImGui::Begin("MenuBar", &p_open, flags);

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Show/Hide Menus")) {

      for (auto& menu : menu_c.windows)
        ImGui::MenuItem(menu.name.c_str(), NULL, &menu.enabled);

      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  ImGui::End();
}

} // namespace game2d