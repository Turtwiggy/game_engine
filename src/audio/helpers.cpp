#include "helpers.hpp"

#include "audio/helpers/openal.hpp"
#include "entt/helpers.hpp"

#include <algorithm>

namespace game2d {

Sound
get_sound(const SINGLETON_AudioComponent& audio, const std::string& tag)
{
  const auto& res = audio.sounds;
  auto it = std::find_if(res.begin(), res.end(), [&tag](const auto& a) { return a.tag == tag; });
  return (*it);
};

void
close_audio(entt::registry& r)
{
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  // delete sources
  for (auto [entity, source] : r.view<AudioSource>().each())
    alDeleteSources(1, &source.source_id);

  // delete buffers
  for (auto i = 0; i < audio.sounds.size(); i++)
    alDeleteBuffers(1, &audio.sounds[i].result);

  // delete context
  auto* context = alcGetCurrentContext();
  if (context != nullptr) {
    auto* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
  }
}

void
open_audio_new_device(entt::registry& r, const std::optional<std::string>& name)
{
  printf("Opening audio....");
  printf("OpenAL version: %s\n", alGetString(AL_VERSION));
  printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
  printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));

  // init audio
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  // create context
  auto* context = alcGetCurrentContext();

  ALCdevice* device;
  if (name != std::nullopt)
    device = alcOpenDevice(name.value().c_str());
  else
    device = alcOpenDevice(0); // default device

  context = alcCreateContext(device, 0);

  if (context == 0 || alcMakeContextCurrent(context) == ALC_FALSE) {
    if (context != 0) {
      fprintf(stdout, "destroyed context!\n");
      alcDestroyContext(context);
    }
    alcCloseDevice(device);
    fprintf(stderr, "Audio could not set a context!\n");
    return;
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
};

void
process_audio_added(entt::registry& r)
{
  close_audio(r);
  for (const auto& device : audio::list_playback_devices()) {
    open_audio_new_device(r, { device }); // set first as default
    break;
  }
};

void
process_audio_removed(entt::registry& r)
{
  close_audio(r);
  for (const auto& device : audio::list_playback_devices()) {
    open_audio_new_device(r, { device }); // set first as default
    break;
  }
};

void
stop_all_audio(entt::registry& r)
{
  const auto& audio_view = r.view<AudioSource>();
  for (const auto& [e, source] : audio_view.each()) {
    ALint source_state;
    alGetSourcei(source.source_id, AL_SOURCE_STATE, &source_state);
    if (source_state == AL_PLAYING)
      alSourceStop(source.source_id);
  }
}

} // namespace game2d