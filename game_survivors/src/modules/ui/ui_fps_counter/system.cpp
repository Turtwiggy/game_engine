#include "pch.hpp"

#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"

namespace game2d {

void
update_ui_fps_counter_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  bool show_fps_counter = true;

  auto options_e = get_first<SINGLE_OptionsMenuState>(r);
  if (options_e != entt::null) {
    auto& options_c = r.get<SINGLE_OptionsMenuState>(options_e);
    show_fps_counter |= options_c.open;
  }

#if !defined(_DEBUG)
  if (!show_fps_counter)
    return;
#endif

  const auto& ri = SINGLE_RendererInfo::instance;
  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);

  ImGui::Begin("overlay", NULL, flags);

  const auto pos = glm::vec2{ ri.viewport_size_render_at.x, 0 }; // tr
  ImGui::SetCursorScreenPos(ImVec2{ pos.x - 100, pos.y });
  ImGui::Text("%0.2f FPS", ImGui::GetIO().Framerate);

  ImGui::End();

  ImGui::PopStyleVar();
}

} // namespace game2d