#include "openal.hpp"

// c++ lib headers
#include <iostream>

namespace game2d {

std::vector<std::string>
audio::list_devices(const ALCchar* devices)
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

// https://github.com/kcat/openal-soft/blob/master/examples/common/alhelpers.c
void
audio::init_al()
{
  ALCcontext* context;
  ALCdevice* device;

  device = alcOpenDevice(0); // select the "preferred device"

  if (!device) {
    std::cout << "(Audio) Could not open a device!" << std::endl;
    return;
  }

  context = alcCreateContext(device, 0);

  if (context == 0 || alcMakeContextCurrent(context) == ALC_FALSE) {
    if (context != 0)
      alcDestroyContext(context);
    alcCloseDevice(device);
    std::cout << "(Audio) Could not set a context!" << std::endl;
    return;
  }

  alcMakeContextCurrent(context);

  printf("OpenAL version: %s\n", alGetString(AL_VERSION));
  printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
  printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));
};

void
audio::close_al()
{
  ALCdevice* device;
  ALCcontext* ctx;

  ctx = alcGetCurrentContext();
  if (ctx == 0)
    return;

  device = alcGetContextsDevice(ctx);

  alcMakeContextCurrent(0);
  alcDestroyContext(ctx);
  alcCloseDevice(device);
}

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