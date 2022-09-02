// header
#include "system.hpp"

#include "modules/audio/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "resources/audio.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_editor_system(entt::registry& r)
{
  ImGui::Begin("Editor", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  auto& audio = r.ctx().at<SINGLETON_AudioComponent>();
  ImGui::Text("Audio");

  static bool mute_audio = false;
  ImGui::Text("Music Muted: %i", mute_audio);
  if (ImGui::Button("Mute")) {
    mute_audio = !mute_audio;
    set_music_pause(mute_audio);
  }
  ImGui::Separator();

  const auto& camera = get_main_camera(r);
  ImGui::Text("Camera");
  ImGui::Text("id: %s", std::to_string(static_cast<uint32_t>(camera)).c_str());
  ImGui::Separator();

  ImGui::End();
};