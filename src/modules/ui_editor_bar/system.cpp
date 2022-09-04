// header
#include "system.hpp"

#include "modules/audio/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "resources/audio.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_editor_bar_system(entt::registry& r)
{
  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();

  if (ImGui::BeginMainMenuBar()) {

    if (ImGui::BeginMenu("Audio")) {

      static bool mute_audio = false;
      ImGui::Text("Music muted: %i", mute_audio);
      if (ImGui::MenuItem("Toggle mute")) {
        mute_audio = !mute_audio;
        set_music_pause(mute_audio);
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Camera")) {
      const auto& camera = get_main_camera(r);
      ImGui::Text("Camera");
      ImGui::Text("id: %s", std::to_string(static_cast<uint32_t>(camera)).c_str());
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
};