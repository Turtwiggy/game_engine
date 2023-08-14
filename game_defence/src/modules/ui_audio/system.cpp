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
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::Begin("Audio", NULL, flags);

  if (ImGui::Button("Refresh Devices"))
    audio.refresh_devices = true;

  if (audio.refresh_devices) {
    audio.playback_devices = audio::list_devices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
    audio.captured_devices = audio::list_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
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
        if (context != nullptr) {
          alcMakeContextCurrent(NULL);
          alcDestroyContext(context);
          alcCloseDevice(device);
        }
      }
    } else
      ImGui::Text("No Audio Context Created");
  }
  ImGui::NewLine();

  ImGui::Text("Select Audio Device");
  for (int i = 0; i < audio.playback_devices.size(); i++) {

    const std::string& name = audio.playback_devices[i];
    const ALchar* al_name = name.c_str();
    const std::string label = std::string("Open ") + name;
    if (ImGui::Button(label.c_str())) {

      // cleanup audio
      {
        // delete sources
        for (auto [entity, source] : r.view<AudioSource>().each()) {
          alDeleteSources(1, &source.source_id);
        }

        // delete buffers
        for (int i = 0; i < audio.sounds.size(); i++) {
          alDeleteBuffers(1, &audio.sounds[i].result);
        }

        // delete context
        auto* context = alcGetCurrentContext();
        if (context != nullptr) {
          auto* device = alcGetContextsDevice(context);
          alcMakeContextCurrent(NULL);
          alcDestroyContext(context);
          alcCloseDevice(device);
        }
      }

      // init audio
      {
        // create context
        auto* context = alcGetCurrentContext();
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

        // create buffers
        for (Sound& sound : audio.sounds) {
          const auto& result = audio::load_sound(sound.path);
          sound.result = std::move(result);
        }

        // create sources
        for (auto [entity, source] : r.view<AudioSource>().each()) {
          ALuint source_id;
          alGenSources(1, &source_id);
          source.source_id = source_id;
        }
      }
    }
  }
  ImGui::NewLine();

  ImGui::Text("¬¬ Sounds ¬¬");
  for (int i = 0; i < audio.sounds.size(); i++) {
    Sound& s = audio.sounds[i];
    std::string label = std::string("Play ") + s.tag + std::string("##sound") + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      r.emplace<AudioRequestPlayEvent>(r.create(), s.tag);
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