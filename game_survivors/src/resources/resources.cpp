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

  const auto add_tex = [&ri, &path](const std::string& p, const std::string& sp, bool linear = true) {
    engine::Texture tex;
    tex.path = path + "textures/" + p;
    tex.spritesheet_path = path + "config/" + sp;
    tex.linear = linear;
    ri.user_textures.push_back(tex);
  };

  add_tex("kenneynl_1bit_pack/monochrome_transparent_packed.png", "spritemap_kenneynl.json");
  add_tex("kenneynl_gameicons/gameicons.png", "spritemap_kenneynl_icons.json");
  add_tex("blueberry_dark.png", "spritemap_studio_logo.json");
  add_tex("smoke_fx_5.png", "spritemap_smoke_fx_5.jsonc");
  add_tex("smoke_fx_6.png", "spritemap_smoke_fx_6.jsonc");
  add_tex("custom.png", "spritemap_custom.json");
  add_tex("custom.png", "spritemap_custom.json", false);
  add_tex("animated.png", "spritemap_animated.jsonc");
  // add_tex("spritestack_rhib.png", "spritestack_rhib.json");
  // add_tex("spritestack_pbr.png", "spritestack_pbr.json");
  add_tex("spritestack_dinghy.png", "spritestack_dinghy.json");
  // add_tex("nyan_cat.png", "nyan_cat.json");

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

  audio.sounds.push_back({ "MENU_0", mus_path + "sergepavkin_smooth-waters_quieter.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "SELECT_0", mus_path + "stranger-things-124008.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "UPGRADE_0", mus_path + "pixabay_ninja.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_0", mus_path + "psychronic-antimatter-ancestor.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_1", mus_path + "psychronic-digital-driving-force.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_2", mus_path + "psychronic-fight-for-the-future.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "GAME_3", mus_path + "psychronic-wacky-fight.mp3", SoundType::BACKGROUND });

  // audio.sounds.push_back({ "BUOY_0", path + "marblehead_bell_buoy_nrm_23.mp3", SoundType::BACKGROUND });
  audio.sounds.push_back({ "UI_HOVER_0", sfx_path + "UI_SCI-FI_Tone_Bright_Wet_12_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "UI_SELECT_0", sfx_path + "UI_SCI-FI_Tone_Bright_Wet_25_stereo.wav", SoundType::SFX });
  audio.sounds.push_back({ "XP_0", sfx_path + "UI_SCI-FI_Tone_Bright_Dry_12_stereo_nrm.mp3", SoundType::SFX });

  audio.sounds.push_back({ "BOMB_EXPLOSION_01", sfx_path + "cannon_shot_nrm.mp3", SoundType::SFX });
  audio.sounds.push_back({ "ENEMY_EXPLODER_01", sfx_path + "exploder_explosion_1.mp3", SoundType::SFX });
  audio.sounds.push_back({ "ENEMY_EXPLODER_02", sfx_path + "exploder_explosion_2.mp3", SoundType::SFX });
  audio.sounds.push_back({ "ENEMY_EXPLODER_03", sfx_path + "exploder_explosion_3.mp3", SoundType::SFX });
  audio.sounds.push_back({ "ENEMY_EXPLODER_04", sfx_path + "exploder_explosion_4.mp3", SoundType::SFX });
  audio.sounds.push_back({ "HIT_01", sfx_path + "boat_hit_01.mp3", SoundType::SFX });
  audio.sounds.push_back({ "HIT_02", sfx_path + "boat_hit_02.mp3", SoundType::SFX });
  audio.sounds.push_back({ "HIT_03", sfx_path + "boat_hit_03.mp3", SoundType::SFX });
  audio.sounds.push_back({ "RIFLE_01", sfx_path + "rifle_01.mp3", SoundType::SFX });
  audio.sounds.push_back({ "RIFLE_02", sfx_path + "rifle_02.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SHOTGUN_01", sfx_path + "shotgun_01.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SHOTGUN_02", sfx_path + "shotgun_02.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SHOTGUN_03", sfx_path + "shotgun_03.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SHOTGUN_04", sfx_path + "shotgun_04.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SMG_01", sfx_path + "smg_01.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SMG_02", sfx_path + "smg_02.mp3", SoundType::SFX });
  audio.sounds.push_back({ "SMG_03", sfx_path + "smg_03.mp3", SoundType::SFX });
  audio.sounds.push_back({ "TURRET_01", sfx_path + "drop_turret_no_shot.mp3", SoundType::SFX });
  audio.sounds.push_back({ "TURRET_02", sfx_path + "drop_turret_with_shot.mp3", SoundType::SFX });
  audio.sounds.push_back({ "DRUM_01", sfx_path + "drum_01.mp3", SoundType::SFX });
  audio.sounds.push_back({ "DRUM_02", sfx_path + "drum_02.mp3", SoundType::SFX });
  audio.sounds.push_back({ "DRUM_03", sfx_path + "drum_03.mp3", SoundType::SFX });
  audio.sounds.push_back({ "DRUM_04", sfx_path + "drum_04.mp3", SoundType::SFX });
  audio.sounds.push_back({ "DRUM_05", sfx_path + "drum_05.mp3", SoundType::SFX });
  audio.sounds.push_back({ "CLANK_01", sfx_path + "clank_01.mp3", SoundType::SFX });
  audio.sounds.push_back({ "ISLAND_CANNON_01", sfx_path + "island_cannon_01.mp3", SoundType::SFX });

  // audio.sounds.push_back({ "WIN_01", "8-bit-win-funk-david-renda.wav" });
  // audio.sounds.push_back({ "LOSS_01", "8-bit-loss-david-renda.wav" });

  return audio;
};

} // namespace game2d