#include "system.hpp"

#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "audio/helpers/openal.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"

#include <fmt/core.h>
#include <imgui.h>


namespace game2d {

void
update_ui_audio_system(entt::registry& r)
{
  // todo: process audio device disconnect and connect events?

  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  ImGui::Begin("Audio");

  if (ImGui::Button("Refresh Devices"))
    audio.refresh_devices = true;

  if (audio.refresh_devices) {
    audio.playback_devices = audio::list_playback_devices();
    audio.captured_devices = audio::list_captured_devices();
    audio.refresh_devices = false;
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
  {
    auto* context = alcGetCurrentContext();
    if (context) {
      auto* device = alcGetContextsDevice(context);
      const auto* device_name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
      ImGui::Text("Device: %s", device_name);
      if (ImGui::Button("Close Audio Device")) {
        if (context != nullptr)
          close_audio(r);
      }
    } else
      ImGui::Text("No Audio Context Created");
  }
  ImGui::NewLine();

  ImGui::Text("Select Audio Device");
  for (int i = 0; i < audio.playback_devices.size(); i++) {

    const std::string& name = audio.playback_devices[i];
    const std::string label = std::string("Open ") + name;
    if (ImGui::Button(label.c_str())) {
      close_audio(r);
      open_audio_new_device(r, { name });
    }
  }
  ImGui::NewLine();

  ImGui::Text("¬¬ Sounds ¬¬");
  for (int i = 0; i < audio.sounds.size(); i++) {
    Sound& s = audio.sounds[i];
    std::string label = std::string("Play ") + s.tag + std::string("##sound") + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ s.tag });
    }
  }

  const auto& sources = r.view<AudioSource>();
  ImGui::Text("¬¬ Audio State ¬¬");
  ImGui::Text("Audio Sources: %i", sources.size());

  // a vector of free audio sources, populated every frame
  std::vector<AudioSource> free_audio_sources;

  // state: playing -> free
  for (const auto& [entity, source] : r.view<AudioSource>().each()) {
    ALint source_state;
    alGetSourcei(source.source_id, AL_SOURCE_STATE, &source_state);

    source.state = AudioSourceState::FREE;
    if (source_state == AL_PLAYING)
      source.state = AudioSourceState::PLAYING;

    if (source.state == AudioSourceState::FREE)
      free_audio_sources.push_back(source);
  }
  ImGui::Text("Free Audio Sources: %i", free_audio_sources.size());

  ImGui::End();
}

} // namespace game2d