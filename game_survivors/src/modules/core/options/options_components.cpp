#include "pch.hpp"

#include "options_components.hpp"

#include "engine/app/application.hpp"
#include "engine/app/game_window.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/core/io/io_helpers.hpp"

namespace game2d {

void
center_window(engine::SINGLE_Application& app)
{
  const auto size = app.window.get_size();
  const auto half_size = glm::ivec2{ size.x * 0.5f, size.y * 0.5f };

  const int monitor_idx = SDL_GetWindowDisplayIndex(app.window.get_handle());
  if (monitor_idx < 0) {
    SDL_Log("error: %s", SDL_GetError());
    return; // unable to get...
  }

  SDL_Rect monitor_bounds;
  if (SDL_GetDisplayBounds(monitor_idx, &monitor_bounds) != 0)
    return; // unable to get...

  const int monitor_w = monitor_bounds.w;
  const int monitor_h = monitor_bounds.h;
  const auto center = glm::ivec2{ (monitor_w * 0.5) - half_size.x, (monitor_h * 0.5) - half_size.y };
  app.window.set_position(center.x, center.y);
};

void
Option_AudioMasterVolume::load(engine::SINGLE_Application& app, entt::registry& r)
{
  if (loaded)
    return;
  loaded = true;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));
  const auto on_disk_opt = savefile_get_key(r, enum_as_str);
  auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);

  if (!on_disk_opt.has_value()) {
    data.value = 0.5f;
    audio_c.volume_master = data.value;
    return;
  }
  auto on_disk_val = on_disk_opt.value();

  // Convert the on_disk_val to your representation.
  Audio_OnDisk ondisk_data;
  on_disk_val.get_to<Audio_OnDisk>(ondisk_data);
  data = ondisk_data;

  // update the system.
  audio_c.volume_master = data.value;
};

void
Option_AudioMasterVolume::update(engine::SINGLE_Application& app, entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 0, 10);
  data.value = (float)engine::scale(hindex, 0, 10, 0.0f, 1.0f);

  const auto enum_as_str = std::string(magic_enum::enum_name(option));

  // convert value to on-disk representation
  const nlohmann::json data_as_json = data;
  savefile_put_key(r, enum_as_str, data_as_json);
  savefile_save_disk(r);
  SDL_Log("volume_master updated to: %f", data.value);

  // update the audio system
  auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);
  audio_c.volume_master = data.value;
};

int
Option_AudioMasterVolume::get_hindex(entt::registry& r)
{
  // For AudioMasterVolume, conver the float to index's 0-10.
  return (int)engine::scale(data.value, 0.0f, 1.0f, 0, 10);
};

std::string
Option_AudioMasterVolume::display_val()
{
  return std::format("{}%", (int)(data.value * 100.0f));
};

//
//
//

void
Option_AudioMusicVolume::load(engine::SINGLE_Application& app, entt::registry& r)
{
  if (loaded)
    return;
  loaded = true;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));
  const auto on_disk_opt = savefile_get_key(r, enum_as_str);
  auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);

  if (!on_disk_opt.has_value()) {
    data.value = 0.5f;
    audio_c.volume_music = data.value;
    return;
  }
  auto on_disk_val = on_disk_opt.value();

  // Convert the on_disk_val to your representation.
  Audio_OnDisk ondisk_data;
  on_disk_val.get_to<Audio_OnDisk>(ondisk_data);
  data = ondisk_data;

  // update the system.
  audio_c.volume_music = data.value;
};

void
Option_AudioMusicVolume::update(engine::SINGLE_Application& app, entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 0, 10);
  data.value = (float)engine::scale(hindex, 0, 10, 0.0f, 1.0f);

  const auto enum_as_str = std::string(magic_enum::enum_name(option));

  // convert value to on-disk representation
  const nlohmann::json data_as_json = data;
  savefile_put_key(r, enum_as_str, data_as_json);
  savefile_save_disk(r);
  SDL_Log("volume_music updated to: %f", data.value);

  // update the audio system
  auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);
  audio_c.volume_music = data.value;
};

int
Option_AudioMusicVolume::get_hindex(entt::registry& r)
{
  // For AudioMasterVolume, conver the float to index's 0-10.
  return (int)engine::scale(data.value, 0.0f, 1.0f, 0, 10);
};

std::string
Option_AudioMusicVolume::display_val()
{
  return std::format("{}%", (int)(data.value * 100.0f));
};

//
//
//

void
Option_AudioSFXVolume::load(engine::SINGLE_Application& app, entt::registry& r)
{
  if (loaded)
    return;
  loaded = true;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));
  const auto on_disk_opt = savefile_get_key(r, enum_as_str);
  auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);

  if (!on_disk_opt.has_value()) {
    data.value = 0.25f;
    audio_c.volume_sfx = data.value;
    return;
  }

  auto on_disk_val = on_disk_opt.value();

  // Convert the on_disk_val to your representation.
  Audio_OnDisk ondisk_data;
  on_disk_val.get_to<Audio_OnDisk>(ondisk_data);
  data = ondisk_data;

  // update the system.
  audio_c.volume_sfx = data.value;
};

void
Option_AudioSFXVolume::update(engine::SINGLE_Application& app, entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 0, 10);
  data.value = (float)engine::scale(hindex, 0, 10, 0.0f, 1.0f);

  const auto enum_as_str = std::string(magic_enum::enum_name(option));

  // convert value to on-disk representation
  const nlohmann::json data_as_json = data;
  savefile_put_key(r, enum_as_str, data_as_json);
  savefile_save_disk(r);
  SDL_Log("volume_music updated to: %f", data.value);

  // update the audio system
  auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);
  audio_c.volume_sfx = data.value;
};

int
Option_AudioSFXVolume::get_hindex(entt::registry& r)
{
  // For AudioMasterVolume, conver the float to index's 0-10.
  return (int)engine::scale(data.value, 0.0f, 1.0f, 0, 10);
};

std::string
Option_AudioSFXVolume::display_val()
{
  return std::format("{}%", (int)(data.value * 100.0f));
};

//
//
//

void
Option_VideoScreenMode::load(engine::SINGLE_Application& app, entt::registry& r)
{
  if (loaded)
    return;
  loaded = true;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));
  const auto on_disk_opt = savefile_get_key(r, enum_as_str);

  if (!on_disk_opt.has_value())
    return;
  auto on_disk_val = on_disk_opt.value();

  // Convert the on_disk_val to your representation.
  Video_ScreenModeOnDisk ondisk_data;
  on_disk_val.get_to<Video_ScreenModeOnDisk>(ondisk_data);
  data = ondisk_data;

  // update the system.
  app.window.set_displaymode(data.screen_mode);

  // set window pos
  center_window(app);
};

void
Option_VideoScreenMode::update(engine::SINGLE_Application& app, entt::registry& r, int& hindex)
{
  const int n_options = (int)engine::DisplayMode::count;
  hindex = glm::clamp(hindex, 0, n_options - 1);
  data.screen_mode = (engine::DisplayMode)hindex;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));

  // convert value to on-disk representation
  const nlohmann::json data_as_json = data;
  savefile_put_key(r, enum_as_str, data_as_json);
  savefile_save_disk(r);
  SDL_Log("screen_mode updated to: %i", hindex);

  // update system
  // note: shouldnt update immediately.
  app.window.set_displaymode(data.screen_mode);

  // set window pos
  center_window(app);
};

int
Option_VideoScreenMode::get_hindex(entt::registry& r)
{
  // just directly convert to enum
  return (int)data.screen_mode;
};

std::string
Option_VideoScreenMode::display_val()
{
  const auto enum_val = data.screen_mode;
  const auto enum_str = std::string(magic_enum::enum_name(enum_val));
  return std::format("{}", enum_str);
};

//
//
//

struct Resolution
{
  int x = 1920;
  int y = 1080;
};

const std::vector<Resolution> resolutions{
  { 1280, 720 },
  { 1920, 1080 },
};

void
Option_VideoResolution::load(engine::SINGLE_Application& app, entt::registry& r)
{
  if (loaded)
    return;
  loaded = true;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));
  const auto on_disk_opt = savefile_get_key(r, enum_as_str);

  if (!on_disk_opt.has_value())
    return;
  auto on_disk_val = on_disk_opt.value();

  // Convert the on_disk_val to your representation.
  Video_ResolutionOnDisk ondisk_data;
  on_disk_val.get_to<Video_ResolutionOnDisk>(ondisk_data);
  data = ondisk_data;

  // update the system.
  app.window.set_size({ data.w, data.h });

  // set window pos
  center_window(app);
};

void
Option_VideoResolution::update(engine::SINGLE_Application& app, entt::registry& r, int& hindex)
{
  const int n_options = (int)resolutions.size();
  hindex = glm::clamp(hindex, 0, n_options - 1);
  data.w = resolutions[hindex].x;
  data.h = resolutions[hindex].y;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));

  // convert value to on-disk representation
  const nlohmann::json data_as_json = data;
  savefile_put_key(r, enum_as_str, data_as_json);
  savefile_save_disk(r);
  SDL_Log("resolution updated to: %i", hindex);

  // update system
  app.window.set_size({ data.w, data.h });

  // set window pos
  center_window(app);
};

int
Option_VideoResolution::get_hindex(entt::registry& r)
{
  // convert w and h to idx
  auto find_res = [&](const Resolution& res) { return res.x == data.w && res.y == data.h; };
  auto it = std::find_if(resolutions.begin(), resolutions.end(), find_res);

  if (it == resolutions.end())
    return 0; // hmm

  const auto idx = static_cast<int>(it - resolutions.begin());
  return idx;
};

std::string
Option_VideoResolution::display_val()
{
  return std::format("{}x{}", data.w, data.h);
};

//
//
//

void
Option_VideoVsync::load(engine::SINGLE_Application& app, entt::registry& r)
{
  if (loaded)
    return;
  loaded = true;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));
  const auto on_disk_opt = savefile_get_key(r, enum_as_str);

  if (!on_disk_opt.has_value())
    return;
  auto on_disk_val = on_disk_opt.value();

  // Convert the on_disk_val to your representation.
  Video_VsyncOnDisk ondisk_data;
  on_disk_val.get_to<Video_VsyncOnDisk>(ondisk_data);
  data = ondisk_data;

  // update the system.
  app.window.set_vsync_opengl(data.enabled);
  SDL_Log("(loaded) Vsync: %i", data.enabled);
};

void
Option_VideoVsync::update(engine::SINGLE_Application& app, entt::registry& r, int& hindex)
{
  // clamp hindex to 0 or 1 (false or true)
  hindex = glm::clamp(hindex, 0, 1);
  data.enabled = hindex;

  const auto enum_as_str = std::string(magic_enum::enum_name(option));

  // convert value to on-disk representation
  const nlohmann::json data_as_json = data;
  savefile_put_key(r, enum_as_str, data_as_json);
  savefile_save_disk(r);
  SDL_Log("screen_mode updated to: %i", hindex);

  // update system
  app.window.set_vsync_opengl(data.enabled);
  SDL_Log("(update) Vsync: %i", data.enabled);
};

int
Option_VideoVsync::get_hindex(entt::registry& r)
{
  return (int)data.enabled;
};

std::string
Option_VideoVsync::display_val()
{
  return std::format("{}", data.enabled);
};

/*
ImGui::SeparatorText("Effects");

auto& crt_c = get_first_component<SINGLE_EffectCrt>(r);
ImGui::Checkbox("CRT", &crt_c.enabled);

static bool grid_effect = true;
ImGui::Checkbox("Grid", &grid_effect);
const auto grid_e = get_first<Effect_GridComponent>(r);
if (grid_effect == true && grid_e == entt::null)
  create_empty<Effect_GridComponent>(r);
if (grid_effect == false && grid_e != entt::null)
  r.destroy(grid_e);
*/

} // namespace game2d