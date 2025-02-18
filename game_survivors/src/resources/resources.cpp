#include "resources/resources.hpp"

#include "engine/io/path.hpp"

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

  add_tex("kennynl_1bit_pack/monochrome_transparent_packed.png", "spritemap_kennynl.json");
  add_tex("kennynl_gameicons/gameicons.png", "spritemap_kennynl_icons.json");
  add_tex("blueberry_dark.png", "spritemap_studio_logo.json");
  add_tex("custom.png", "spritemap_custom.json");
  // add_tex("logo.png", "spritemap_text_logo.json");
  // add_tex("worley_noise.png", "spritemap_default_1024.json");
  // add_tex("ship_small_rusty.png", "spritemap_ship_small_rusty.json");
  add_tex("animation_idle.png", "spritemap_animation_idle.json");
  add_tex("junkers.png", "spritemap_junkers.json");
  add_tex("spritestack_dinghy.png", "spritestack_dinghy.json");
  add_tex("spritestack_rhib.png", "spritestack_rhib.json");
  add_tex("spritestack_pbr.png", "spritestack_pbr.json");
  add_tex("spritestack_trimaran.png", "spritestack_trimaran.json");

  return ri;
};

SINGLE_AudioComponent
get_default_audio()
{
  std::string path = engine::get_exe_path_without_exe_name();
  path += "assets/audio/";

  SINGLE_AudioComponent audio;

  audio.sounds.push_back(
    { "SHOTGUN_SHOOT_01", path + "FIREARM_Shotgun_Model_02_Fire_Single_RR1_stereo.wav", SoundType::SFX });

  audio.sounds.push_back({ "SHOTGUN_RELOAD_01", path + "RELOAD_Pump_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "TAKE_DAMAGE_01", path + "GRUNT_Male_Subtle_Hurt_mono.wav", SoundType::SFX });

  audio.sounds.push_back({ "MENU_01", path + "stranger-things-124008.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_01", path + "lady-of-the-80.mp3", SoundType::BACKGROUND });
  // audio.sounds.push_back({ "GAME_02", path + "lady-of-the-80.mp3", SoundType::BACKGROUND });

  audio.sounds.push_back({ "UI_HOVER_01", path + "UI_SCI-FI_Tone_Bright_Wet_12_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "UI_SELECT_01", path + "UI_SCI-FI_Tone_Bright_Wet_25_stereo.wav", SoundType::SFX });

  audio.sounds.push_back({ "XP_01", path + "UI_SCI-FI_Tone_Bright_Dry_12_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "XP_02", path + "UI_SCI-FI_Tone_Bright_Dry_13_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "XP_03", path + "UI_SCI-FI_Tone_Bright_Dry_14_stereo.wav", SoundType::SFX });

  audio.sounds.push_back({ "SHOOT_01", path + "FIREARM_Shotgun_Model_02_Fire_Single_RR1_stereo.wav", SoundType::SFX });

  // audio.sounds.push_back({ "COMBAT_01", path + ".mp3", SoundType::BACKGROUND });
  // audio.sounds.push_back({ "WIN_01", "8-bit-win-funk-david-renda.wav" });
  // audio.sounds.push_back({ "LOSS_01", "8-bit-loss-david-renda.wav" });

  return audio;
};

} // namespace game2d