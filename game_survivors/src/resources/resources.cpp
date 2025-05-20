#include "pch.hpp"

#include "resources/resources.hpp"

#include "engine/io/path.hpp"
#include "steam/isteaminput.h"

namespace game2d {

SINGLE_RendererInfo
get_default_textures()
{
  SINGLE_RendererInfo ri;

  std::string path = engine::get_exe_path_without_exe_name();
  path += "assets/";

  const auto add_tex = [&ri, &path](const std::string& p, const std::string& sp) {
    Texture tex;
    tex.path = path + "textures/" + p;
    tex.spritesheet_path = path + "config/" + sp;
    ri.user_textures.push_back(tex);
  };

  add_tex("kenneynl_1bit_pack/monochrome_transparent_packed.png", "spritemap_kenneynl.json");
  add_tex("kenneynl_gameicons/gameicons.png", "spritemap_kenneynl_icons.json");
  add_tex("blueberry_dark.png", "spritemap_studio_logo.json");
  add_tex("smoke_fx_5.png", "spritemap_smoke_fx_5.jsonc");
  add_tex("smoke_fx_6.png", "spritemap_smoke_fx_6.jsonc");
  add_tex("custom.png", "spritemap_custom.json");
  add_tex("grime_bar.png", "spritemap_grime_bar.json");
  add_tex("snake.png", "spritemap_snake.json");
  add_tex("spritestack_rhib.png", "spritestack_rhib.json");
  add_tex("spritestack_pbr.png", "spritestack_pbr.json");
  add_tex("spritestack_dinghy.png", "spritestack_dinghy.json");

  // const ESteamInputGlyphSize size = k_ESteamInputGlyphSize_Medium;
  // for (int i = 0; i < k_EInputActionOrigin_Count; i++) {
  //   const auto origin = magic_enum::enum_cast<EInputActionOrigin>(i).value();
  //   const char* local_glyph_path = SteamInput()->GetGlyphPNGForActionOrigin(origin, size, 0);
  //   if (!local_glyph_path)
  //     continue;
  //   SDL_Log("steam glpyh path = %s", local_glyph_path);
  //   Texture tex;
  //   tex.path = local_glyph_path;
  //   ri.user_textures.push_back(tex);
  // }

  return ri;
};

SINGLE_AudioComponent
get_default_audio()
{
  std::string path = engine::get_exe_path_without_exe_name();
  path += "assets/audio/";
  std::string sfx_path = path + "sfx/";
  std::string mus_path = path + "music/";
  std::string amb_path = path + "ambience/";

  SINGLE_AudioComponent audio;

  audio.sounds.push_back({ "WATER_AMBIENCE_0",
                           amb_path + "Water, Wave, Ocean, Beach Waves, Medium, Lapping  SND0009.mp3",
                           SoundType::BACKGROUND });

  // audio.sounds.push_back({ "BUOY_0", path + "marblehead_bell_buoy_nrm_23.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back(
    { "SHOTGUN_SHOOT_0", sfx_path + "FIREARM_Shotgun_Model_02_Fire_Single_RR1_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "SHOTGUN_RELOAD_0", sfx_path + "RELOAD_Pump_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "TAKE_DAMAGE_0", sfx_path + "GRUNT_Male_Subtle_Hurt_mono.wav", SoundType::SFX });
  audio.sounds.push_back({ "UI_HOVER_0", sfx_path + "UI_SCI-FI_Tone_Bright_Wet_12_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "UI_SELECT_0", sfx_path + "UI_SCI-FI_Tone_Bright_Wet_25_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "XP_0", sfx_path + "UI_SCI-FI_Tone_Bright_Dry_12_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "SHOOT_0", sfx_path + "FIREARM_Shotgun_Model_02_Fire_Single_RR1_stereo.wav", SoundType::SFX });

  audio.sounds.push_back({ "MENU_0", mus_path + "sergepavkin_smooth-waters_quieter.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "SELECT_0", mus_path + "stranger-things-124008.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "UPGRADE_0", mus_path + "pixabay_ninja.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_0", mus_path + "psychronic-antimatter-ancestor.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_1", mus_path + "psychronic-digital-driving-force.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_2", mus_path + "psychronic-fight-for-the-future.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_3", mus_path + "psychronic-wacky-fight.mp3", SoundType::BACKGROUND });

  // audio.sounds.push_back({ "WIN_01", "8-bit-win-funk-david-renda.wav" });
  // audio.sounds.push_back({ "LOSS_01", "8-bit-loss-david-renda.wav" });

  return audio;
};

} // namespace game2d