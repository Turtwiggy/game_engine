#include "openal.hpp"

// c++ lib headers
#include <iostream>

namespace game2d {

// void
// list_audio_devices(const ALCchar* devices)
// {
//   const ALCchar *device = devices, *next = devices + 1;
//   size_t len = 0;
//   fprintf(stdout, "(Audio) Devices list:\n");
//   fprintf(stdout, "----------\n");
//   while (device && *device != '\0' && next && *next != '\0') {
//     fprintf(stdout, "%s\n", device);
//     len = strlen(device);
//     device += (len + 1);
//     next += (len + 2);
//   }
//   fprintf(stdout, "----------\n");
// }

void
audio::init_al()
{
  const ALchar* name;
  ALCdevice* device;
  ALCcontext* context;

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

  name = 0;
  if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
    name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
  if (!name || alcGetError(device) != AL_NO_ERROR)
    name = alcGetString(device, ALC_DEVICE_SPECIFIER);

  printf("OpenAL version: %s\n", alGetString(AL_VERSION));
  printf("OpenAL vendor: %s\n", alGetString(AL_VENDOR));
  printf("OpenAL renderer: %s\n", alGetString(AL_RENDERER));
  std::cout << "(Audio) Opened: " << name << std::endl;
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
  // Audio source state.
  unsigned char* data = NULL;
  unsigned int size = 0;
  unsigned int offset = 0;
  unsigned int channels = 0;
  unsigned int frequency = 0;
  unsigned int bits = 0;
  ALenum format = 0;
  ALuint source = 0;

  {
    FILE* fp = fopen(filename.c_str(), "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = (unsigned char*)malloc(size);
    fread(data, size, 1, fp);
    fclose(fp);
  }

  offset = 12; // ignore the RIFF header
  offset += 8; // ignore the fmt header
  offset += 2; // ignore the format type

  channels = data[offset + 1] << 8;
  channels |= data[offset];
  offset += 2;
  printf("Channels: %u\n", channels);

  frequency = data[offset + 3] << 24;
  frequency |= data[offset + 2] << 16;
  frequency |= data[offset + 1] << 8;
  frequency |= data[offset];
  offset += 4;
  printf("Frequency: %u\n", frequency);

  offset += 6; // ignore block size and bps

  bits = data[offset + 1] << 8;
  bits |= data[offset];
  offset += 2;
  printf("Bits: %u\n", bits);

  format = AL_NONE;
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
  offset += 8; // ignore the data chunk

  /* Decode the whole audio file to a buffer. */

  ALuint buffer;
  alGenBuffers(1, &buffer);
  alBufferData(buffer, format, &data[offset], size - offset, frequency);
  free(data);

  // Check if an error occured, and clean up if so.
  ALenum err = alGetError();
  if (err != AL_NO_ERROR) {
    std::cout << "OpenAL Error: " << alGetString(err) << std::endl;
    if (buffer && alIsBuffer(buffer))
      alDeleteBuffers(1, &buffer);
    return 0;
  }

  std::cout << "(Audio) Loaded Sound:" << filename << std::endl;
  return buffer;
}

void
audio::play_sound(ALint source_id)
{
  alSourceStop(source_id);
  alSourcePlay(source_id);
};

} // namespace game2d