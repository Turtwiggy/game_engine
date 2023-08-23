#pragma once

#include "SDL2/SDL_audio.h"
#include <AL/al.h>
#include <AL/alc.h>

#include <string>
#include <vector>

namespace game2d {

namespace audio {

// https://openal.org/documentation/OpenAL_Programmers_Guide.pdf

// The usage of OpenAL revolves around 3 fundemental objects
// buffers, sources, and a listener

// sources can be positioned and played.
// source sound is determined position and orientation relative to the listener object.

// when initializing openal, at least one device has to be opened.
// within that device, at least one context will be created.
// each source can have one or more buffers attached to it.
// buffer objects are not part of a specific context, they are shared among all contexts on one device.

// device enumeration
// the function to open a device, alcOpenDevice, takes a strin as an input
// The string should contain either a name of OpenAL rendering device, or NULL to request the default device.

/* LoadBuffer loads the named audio file into an OpenAL buffer object, and
 * returns the new buffer ID. */
ALuint
load_sound(const std::string& filename);

void
play_sound(ALint source_id);

std::vector<std::string>
list_playback_devices();

std::vector<std::string>
list_captured_devices();

} // namespace audio

} // namespace game2d