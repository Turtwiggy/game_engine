// header
#include "sprites/helpers.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"

// other libs
#include "entt/entt.hpp"
#include <nlohmann/json.hpp>

// std libs
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace game2d {
using json = nlohmann::json;

struct Frame
{
  int x = 0;
  int y = 0;
};

void
load_sprites(SINGLE_Animations& anims, const std::string path)
{
  auto& sprites = anims.animations;

  std::cout << "loading sprite config: " << path << "\n";
  std::ifstream f(path);
  json data = json::parse(f);

  auto ss = data["spritesheet"];
  // std::string name = ss["name"];
  // int px_total = ss["px_total"];
  // int py_total = ss["py_total"];
  // int px = ss["px"];
  // int py = ss["py"];
  // int nx = ss["nx"];
  // int ny = ss["ny"];

  auto s = data["sprites"];
  for (auto& el : s) {
    auto name = el["name"];
    auto frames = el["frames"];

    SpriteAnimation anim;
    anim.name = name;

    for (auto& frame : frames) {
      SpritePosition pos;
      pos.x = frame["x"];
      pos.y = frame["y"];
      if (frame.contains("w"))
        pos.w = frame["w"];
      if (frame.contains("h"))
        pos.h = frame["h"];
      anim.animation_frames.push_back(pos);
    }

    if (el.contains("angle"))
      anim.angle_degrees = el["angle"];

    sprites.push_back(anim);
  }
};

SpriteAnimation
find_animation(const SINGLE_Animations& anims, const std::string name)
{
  auto& sprites = anims.animations;
  auto s = std::find_if(sprites.begin(), sprites.end(), [&name](const SpriteAnimation& spr) { return spr.name == name; });
  if (s != std::end(sprites))
    return *s;
  else {
    std::cerr << "CONFIG ERROR: sprite not found: " << name << "\n";
    exit(1); // explode!
  }
}

void
set_sprite(entt::registry& r, const entt::entity& e, const std::string& sprite)
{
  auto& anims = get_first_component<SINGLE_Animations>(r);

  // set anim
  const auto anim = find_animation(anims, sprite);
  auto& sprc = r.get<SpriteComponent>(e);
  sprc.tex_pos.x = anim.animation_frames[0].x;
  sprc.tex_pos.y = anim.animation_frames[0].y;
};

glm::ivec2
set_sprite_custom(entt::registry& r, const entt::entity& e, const std::string& sprite)
{
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto anim = find_animation(anims, sprite);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto tex = search_for_texture_by_path(ri, "bargame");

  auto& sc = r.get<SpriteComponent>(e);
  sc.tex_unit = tex->unit;
  sc.total_sx = 32;
  sc.total_sy = 32;
  sc.tex_pos.x = anim.animation_frames[0].x;
  sc.tex_pos.y = anim.animation_frames[0].y;
  sc.tex_pos.w = anim.animation_frames[0].w;
  sc.tex_pos.h = anim.animation_frames[0].h;

  return { anim.animation_frames[0].w, anim.animation_frames[0].h };
};

} // namespace game2d
