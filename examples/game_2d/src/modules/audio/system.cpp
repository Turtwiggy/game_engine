#include "modules/audio/system.hpp"

// components
#include "components/singleton_resources.hpp"
#include "modules/audio/components.hpp"

// engine

// other lib headers
#include <SDL_mixer.h>

// c lib headers
#include <iostream>
#include <stdio.h>

void
game2d::init_audio_system(entt::registry& registry)
{
  printf("Init audio system... \n");
  auto& am = registry.ctx_or_set<SINGLETON_AudioComponent>(SINGLETON_AudioComponent());

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
    printf("Mix_OpenAudio: %s\n", Mix_GetError());
    exit(2);
  }

  int result = 0;
  auto flags = MIX_INIT_OGG | MIX_INIT_MP3;
  auto init = Mix_Init(flags);
  if (init != flags) {
    printf("Could not initialize mixer(result : % d).\n", result);
    printf("Mix_Init : % s\n", Mix_GetError());
    exit(2);
  }

  // load some audio!
  am.bgm["main_menu"] = Mix_LoadWAV("assets/2d_game/audio/BGM/BGM_ARPG.ogg");
  am.bgm["ambience"] = Mix_LoadWAV("assets/2d_game/audio/SFX/SFX_AmbienceFar.ogg");
  am.sfx["movement_footsteps"].push_back(Mix_LoadWAV("assets/2d_game/audio/SFX/SFX_Footstep01.ogg"));
  am.sfx["movement_footsteps"].push_back(Mix_LoadWAV("assets/2d_game/audio/SFX/SFX_Footstep02.ogg"));
  am.sfx["movement_footsteps"].push_back(Mix_LoadWAV("assets/2d_game/audio/SFX/SFX_Footstep03.ogg"));
  am.sfx["movement_footsteps"].push_back(Mix_LoadWAV("assets/2d_game/audio/SFX/SFX_Footstep04.ogg"));

  Mix_Volume(-1, MIX_MAX_VOLUME / 4); // try not to blast ears

  // play background music
  Mix_Volume(0, MIX_MAX_VOLUME / 8);
  Mix_PlayChannel(0, am.bgm["main_menu"], -1);
  Mix_PlayChannel(1, am.bgm["ambience"], -1);
};

void
game2d::update_audio_system(entt::registry& registry, engine::Application& app, float dt)
{
  auto& am = registry.ctx<SINGLETON_AudioComponent>();
  auto& res = registry.ctx<SINGLETON_ResourceComponent>();

  if (app.get_input().get_key_down(SDL_SCANCODE_W) || app.get_input().get_key_down(SDL_SCANCODE_A) ||
      app.get_input().get_key_down(SDL_SCANCODE_S) || app.get_input().get_key_down(SDL_SCANCODE_D)) {

    // choose a random footstep
    int size = am.sfx["movement_footsteps"].size();
    auto sfx_idx = engine::rand_det_s(res.rnd.rng, 0, size);
    Mix_Chunk* sfx = am.sfx["movement_footsteps"][sfx_idx];

    int loops = 0;
    Mix_PlayChannel(-1, sfx, loops);
  }
};