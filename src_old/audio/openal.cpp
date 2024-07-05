#include "openal.hpp"

// c++ lib headers
#include <al.h>
#include <alc.h>
#include <fmt/core.h>

#include "SDL2/SDL_audio.h"
#include "audio/components.hpp"
#include "entt/helpers.hpp"

namespace game2d {

namespace audio {

namespace openal {

std::vector<std::string>
list_devices(const ALCchar* devices)
{
  const ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) {
    fmt::println("Audio device enumeration not supported");
    return {};
  }

  std::vector<std::string> devices_vec;
  const ALCchar *device = devices, *next = devices + 1;
  size_t len = 0;
  while (device && *device != '\0' && next && *next != '\0') {
    devices_vec.push_back(std::string(device));
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }

  return devices_vec;
}

std::vector<std::string>
list_playback_devices()
{
  return list_devices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
};

std::vector<std::string>
list_captured_devices()
{
  return list_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
};

ALuint
load_wav(const std::string& filename)
{
  SDL_AudioSpec wav_spec;
  Uint32 wav_length;
  Uint8* wav_buffer;

  if (SDL_LoadWAV(filename.c_str(), &wav_spec, &wav_buffer, &wav_length) == NULL) {
    fmt::println("Could not open .wav: {}", SDL_GetError());
    exit(1); // if fail to load audio, explode!
  }
  fmt::println("(Audio) Loaded .wav: {}", filename);

  // const auto& sdl_format = wav_spec.format;
  const int bits = SDL_AUDIO_BITSIZE(wav_spec.format);
  const int channels = wav_spec.channels;

  // fmt::println("bits: " << bits << " ";
  // fmt::println("freq: " << wav_spec.freq << " ";
  // fmt::println("chan: " << channels << " ";
  // fmt::println("format: " << wav_spec.format << " ";
  // fmt::println("samples: " << wav_spec.samples << "\n";

  ALuint buffer;
  alGenBuffers(1, &buffer);

  ALenum format = AL_NONE;
  if (bits == 8) {
    if (channels == 1) {
      format = AL_FORMAT_MONO8;
    } else if (channels == 2) {
      format = AL_FORMAT_STEREO8;
    }
  } else if (bits == 16) {
    if (channels == 1) {
      format = AL_FORMAT_MONO16;
    } else if (channels == 2) {
      format = AL_FORMAT_STEREO16;
    }
  }
  alBufferData(buffer, format, wav_buffer, wav_length, wav_spec.freq);
  SDL_FreeWAV(wav_buffer);

  // Check if an error occured, and clean up if so.
  ALenum err = alGetError();
  if (err != AL_NO_ERROR) {
    fmt::println("OpenAL Error: {}", alGetString(err));
    if (buffer && alIsBuffer(buffer))
      alDeleteBuffers(1, &buffer);
    return 0;
  }

  fmt::println("(Audio) Successfully config sound: {}", filename);
  return buffer;
};

// based on:
// https://github.com/emscripten-core/emscripten/blob/main/test/openal_buffers.c
ALuint
load_sound(const std::string& filename)
{
  if (filename.find(".wav") != std::string::npos)
    return load_wav(filename);

  // music not loaded properly
  return 0;
};

void
play_sound(ALint source_id)
{
  alSourceStop(source_id);
  alSourcePlay(source_id);
};

void
close_audio(entt::registry& r)
{
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

  // delete sources
  // for (auto [entity, source] : r.view<AudioSource>().each())
  //   alDeleteSources(1, &source.source_id);

  // delete buffers
  // for (auto i = 0; i < audio.sounds.size(); i++)
  //   alDeleteBuffers(1, &audio.sounds[i].result);

  // delete context
  auto* context = alcGetCurrentContext();
  if (context != nullptr) {
    auto* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
  }
};

void
open_audio_new_device(entt::registry& r, const std::optional<std::string>& name)
{
  fmt::println("Opening audio....");

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
      fmt::println("(Audio) destroyed context");
      alcDestroyContext(context);
    }
    alcCloseDevice(device);
    fmt::println("(Audio) could not set a context");
    return;
  }

  const ALchar* openal_ver = alGetString(AL_VERSION);
  const ALchar* openal_ven = alGetString(AL_VENDOR);
  const ALchar* openal_ren = alGetString(AL_RENDERER);
  fmt::println("OpenAL version: {}", reinterpret_cast<const char*>(openal_ver));
  fmt::println("OpenAL vendor:  {}", reinterpret_cast<const char*>(openal_ven));
  fmt::println("OpenAL renderer:{}", reinterpret_cast<const char*>(openal_ren));

  // create buffers
  for (Sound& sound : audio.sounds) {
    const auto& result = load_sound(sound.path);
    // sound.result = std::move(result);
  }

  // create sources
  for (auto [entity, source] : r.view<AudioSource>().each()) {
    ALuint source_id;
    alGenSources(1, &source_id);
    // source.source_id = source_id;
  }
};

void
process_audio_added(entt::registry& r)
{
  close_audio(r);
  for (const auto& device : list_playback_devices()) {
    open_audio_new_device(r, { device }); // set first as default
    break;
  }
};

void
process_audio_removed(entt::registry& r)
{
  close_audio(r);
  for (const auto& device : list_playback_devices()) {
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
    // alGetSourcei(source.source_id, AL_SOURCE_STATE, &source_state);
    // if (source_state == AL_PLAYING)
    //   alSourceStop(source.source_id);
  }
};

} // namespace openal

} // namespace audio

} // namespace game2d