#include "openal.hpp"

// c++ lib headers
#include <iostream>

namespace game2d {

std::vector<std::string>
list_devices(const ALCchar* devices)
{
  ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) {
    fprintf(stdout, "Audio device enumeration not supported");
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
audio::list_playback_devices()
{
  return list_devices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
};

std::vector<std::string>
audio::list_captured_devices()
{
  return list_devices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
};

// based on:
// https://github.com/emscripten-core/emscripten/blob/main/test/openal_buffers.c
ALuint
audio::load_sound(const std::string& filename)
{
  SDL_AudioSpec wav_spec;
  Uint32 wav_length;
  Uint8* wav_buffer;
  if (SDL_LoadWAV(filename.c_str(), &wav_spec, &wav_buffer, &wav_length) == NULL) {
    fprintf(stderr, "Could not open .wav: %s\n", SDL_GetError());
  }
  std::cout << "(Audio) Loaded Sound:" << filename << "\n";

  const int bits = SDL_AUDIO_BITSIZE(wav_spec.format);
  const int channels = wav_spec.channels;

  // std::cout << "bits: " << bits << " ";
  // std::cout << "freq: " << wav_spec.freq << " ";
  // std::cout << "chan: " << channels << " ";
  // std::cout << "format: " << wav_spec.format << " ";
  // std::cout << "samples: " << wav_spec.samples << "\n";

  ALuint buffer;
  alGenBuffers(1, &buffer);

  // const auto& sdl_format = wav_spec.format;
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
    std::cout << "OpenAL Error: " << alGetString(err) << "\n";
    if (buffer && alIsBuffer(buffer))
      alDeleteBuffers(1, &buffer);
    return 0;
  }

  std::cout << "(Audio) Successfully config Sound:" << filename << "\n";
  return buffer;
};

void
audio::play_sound(ALint source_id)
{
  alSourceStop(source_id);
  alSourcePlay(source_id);
};

} // namespace game2d