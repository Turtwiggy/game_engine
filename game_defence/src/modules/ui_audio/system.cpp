#include "system.hpp"

#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "audio/helpers/openal.hpp"
#include "entt/helpers.hpp"

#include <imgui.h>
#include <iostream>

namespace game2d {

void
update_ui_audio_system(entt::registry& r)
{
  // todo: process audio device disconnect and connect events?

  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiDockNodeFlags_AutoHideTabBar;

  ImGui::Begin("Audio", NULL, flags);

  if (ImGui::Button("Refresh Devices"))
    audio.refresh_devices = true;

  if (audio.refresh_devices) {
    audio.playback_devices = audio::list_devices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
    audio.captured_devices = audio::list_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
  }

  ImGui::Text("Audio Devices");
  for (int i = 0; i < audio.playback_devices.size(); i++)
    ImGui::Text("Available: %s", audio.playback_devices[i].c_str());
  ImGui::NewLine();

  ImGui::Text("Captured Devices");
  for (int i = 0; i < audio.captured_devices.size(); i++)
    ImGui::Text("captured_devices: %s", audio.captured_devices[i].c_str());
  ImGui::NewLine();

  ImGui::Text("Context Audio Device");
  auto* context = alcGetCurrentContext();
  if (context) {
    auto* device = alcGetContextsDevice(context);
    const auto* device_name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    ImGui::Text("Device: %s", device_name);
    if (ImGui::Button("Close Audio Device")) {
      if (context != nullptr) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
      }
    }
  } else
    ImGui::Text("No Audio Context Created");
  ImGui::NewLine();

  ImGui::Text("Select Audio Device");
  for (int i = 0; i < audio.playback_devices.size(); i++) {

    const std::string& name = audio.playback_devices[i];
    ImGui::Text("Device: %s", name.c_str());
    ImGui::SameLine();

    const ALchar* al_name = name.c_str();
    const std::string label = std::string("Open Devices##") + name;
    if (ImGui::Button(label.c_str())) {

      // Delete old context
      if (context != nullptr) {
        auto* device = alcGetContextsDevice(context);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
      }

      // Create new context with new device
      ALCdevice* device = alcOpenDevice(al_name);
      context = alcCreateContext(device, 0);
      if (context == 0 || alcMakeContextCurrent(context) == ALC_FALSE) {
        if (context != 0) {
          fprintf(stdout, "destroyed context!\n");
          alcDestroyContext(context);
        }
        alcCloseDevice(device);
        fprintf(stdout, "Audio could not set a context!\n");
      }

      auto success = alcMakeContextCurrent(context);
    }
  }
  ImGui::NewLine();

  for (int i = 0; i < audio.sounds.size(); i++) {
    Sound& s = audio.sounds[i];
    ImGui::Text("¬¬ Sound ¬¬");
    ImGui::Text("tag: %s", s.tag.c_str());
    // ImGui::Text("path: %s", s.path.c_str());

    std::string label = std::string("Play Sound##sound") + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      r.emplace<AudioRequestPlayEvent>(r.create(), s.tag);
    }
  }

  ImGui::End();
}

}