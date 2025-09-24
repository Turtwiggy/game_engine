#pragma once

#ifdef __cplusplus

#include <cmath>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
#include "windows.h"
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
#endif

#include <box2d/box2d.h>

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <magic_enum.hpp>

// clang-format off
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
// clang-format on

#include <steam/isteaminput.h>
#include <steam/steam_api.h>
#include <steam/steam_api_common.h>
#include <steam/steam_api_flat.h>

#if defined(_DEBUG)
#include <gtest/gtest.h>
#include <tracy/Tracy.hpp>
#endif

#endif