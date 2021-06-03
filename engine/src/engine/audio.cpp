// your header
#include "engine/audio.hpp"

// c++ lib headers
#include <iostream>

// other project headers
#include <AL/alext.h>
#include <sndfile.h>

namespace fightingengine {

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

ALuint
audio::load_sound(const std::string& filename)
{
  ALenum err, format;
  ALuint buffer;
  SNDFILE* sndfile;
  SF_INFO sfinfo;
  sf_count_t num_frames;
  ALsizei num_bytes;

  /* Open the audio file and check that it's usable. */
  sndfile = sf_open(filename.c_str(), SFM_READ, &sfinfo);
  if (!sndfile) {
    std::cout << "could not open audio in: " << filename << ": " << sf_strerror(sndfile) << std::endl;
    return 0;
  }
  if (sfinfo.frames < 1 || sfinfo.frames > (sf_count_t)(INT_MAX / sizeof(short)) / sfinfo.channels) {
    std::cout << "Bad sample count in " << filename << ": " << sfinfo.frames << std::endl;
    sf_close(sndfile);
    return 0;
  }

  /* Get the sound format, and figure out the OpenAL format */
  format = AL_NONE;
  if (sfinfo.channels == 1)
    format = AL_FORMAT_MONO16;
  else if (sfinfo.channels == 2)
    format = AL_FORMAT_STEREO16;
  else if (sfinfo.channels == 3) {
    if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, 0, 0) == SF_AMBISONIC_B_FORMAT)
      format = AL_FORMAT_BFORMAT2D_16;
  } else if (sfinfo.channels == 4) {
    if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, 0, 0) == SF_AMBISONIC_B_FORMAT)
      format = AL_FORMAT_BFORMAT3D_16;
  }
  if (!format) {
    std::cout << "Unsupported channel count " << sfinfo.channels << std::endl;
    sf_close(sndfile);
    return 0;
  }

  /* Decode the whole audio file to a buffer. */

  short* membuf = (short*)malloc(static_cast<size_t>((sfinfo.frames * sfinfo.channels) * sizeof(short)));

  num_frames = sf_readf_short(sndfile, membuf, sfinfo.frames);
  if (num_frames < 1) {
    free(membuf);
    sf_close(sndfile);
    std::cout << "Failed to read samples in " << filename << ": " << num_frames << std::endl;
    return 0;
  }
  num_bytes = (ALsizei)(num_frames * sfinfo.channels) * (ALsizei)sizeof(short);

  /* Buffer the audio data into a new buffer object, then free the data and
   * close the file.
   */
  buffer = 0;
  alGenBuffers(1, &buffer);
  alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

  free(membuf);
  sf_close(sndfile);

  /* Check if an error occured, and clean up if so. */
  err = alGetError();
  if (err != AL_NO_ERROR) {
    std::cout << "OpenAL Error: " << alGetString(err) << std::endl;
    if (buffer && alIsBuffer(buffer))
      alDeleteBuffers(1, &buffer);
    return 0;
  }

  return buffer;
}

void
audio::play_sound(ALint source_id)
{
  alSourceStop(source_id);
  alSourcePlay(source_id);
};

} // namespace fightingengine