#include "pch.hpp"

#include "system.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/audio/audio_helpers.hpp"
#include "engine/audio/audio_system.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"

namespace game2d {

void
update_ui_audio_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // todo: process audio device disconnect and connect events?

  const auto audio_e = get_first<SINGLE_AudioComponent>(r);
  if (audio_e == entt::null)
    return;
  auto& audio = get_first_component<SINGLE_AudioComponent>(r);

  ImGui::Begin("Audio");

  if (ImGui::Button("Refresh Devices"))
    audio.refresh_devices = true;

  if (audio.refresh_devices) {
    audio.devices = audio::sdl_mixer::list_devices();
    audio.refresh_devices = false;
  }

  ImGui::Text("Audio Devices");
  for (size_t i = 0; i < audio.devices.size(); i++)
    ImGui::Text("Available: %s", audio.devices[i].c_str());
  ImGui::NewLine();

  // ImGui::Text("Captured Devices");
  // ImGui::Text("captured_device_id: %i", audio.captured_device_id);
  ImGui::NewLine();

  ImGui::Text("Select Audio Device");
  for (size_t i = 0; i < audio.devices.size(); i++) {

    const std::string& name = audio.devices[i];
    const std::string label = std::string("Open ") + name;

    if (ImGui::Button(label.c_str())) {

      const char* device_name = SDL_GetAudioDeviceName(i, 0);
      if (device_name == nullptr) {
        SDL_Log("%s", std::format("No Default Audio Device enabled. Not loading sounds.").c_str());
        audio.loaded = true;
        return; // no available devices
      }
      SDL_Log("%s", std::format("Using audiodevice: {}", device_name).c_str());

      refresh_audio(r, std::string{ device_name });
    }
  }
  ImGui::NewLine();

  ImGui::Text("¬¬ Sounds ¬¬");
  for (size_t i = 0; i < audio.sounds.size(); i++) {
    Sound& s = audio.sounds[i];
    std::string label = std::string("Play ") + s.tag + std::string("##sound") + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ s.tag });
    }
  }

  const auto& sources = r.view<AudioSource>();
  ImGui::Text("¬¬ Audio State ¬¬");
  ImGui::Text("Audio Sources: %zu", sources.size());

  for (const auto& [e, source] : sources.each()) {
    ImGui::Text("channel %i ", source.channel);
    ImGui::SameLine();

    const auto type_name = std::string(magic_enum::enum_name(source.state));
    ImGui::Text("%s ", type_name.c_str());

    ImGui::SameLine();
    ImGui::Text("%s", source.sound.c_str());

    ImGui::SameLine();
    const auto vol = Mix_Volume(source.channel, -1);
    ImGui::Text(", Vol: %i", vol);

    //
  }

  ImGui::End();
}

} // namespace game2d