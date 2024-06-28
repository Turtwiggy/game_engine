// header
#include "sprites/helpers.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"

// other libs
#include "entt/entt.hpp"
#include <nlohmann/json.hpp>

// std libs
#include <fstream>
#include <print>
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
  std::println("loading sprite config: {}", path);
  std::ifstream f(path);
  json data = json::parse(f);

  auto ss = data["spritesheet"];
  Spritesheet spritesheet;
  spritesheet.name = ss["name"];
  spritesheet.path = path;
  spritesheet.px_total = ss["px_total"];
  spritesheet.py_total = ss["py_total"];
  spritesheet.px = ss["px"];
  spritesheet.py = ss["py"];
  spritesheet.nx = ss["nx"];
  spritesheet.ny = ss["ny"];

  std::vector<SpriteAnimation> sprites;

  auto s = data["sprites"]; // consumes
  for (auto& el : s) {
    const auto name = el["name"];
    const auto frames = el["frames"];

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

  // append spritesheet global info
  auto info = std::make_pair(spritesheet, sprites);
  anims.animations.push_back(std::move(info));
};

std::pair<Spritesheet, SpriteAnimation>
find_animation(const SINGLE_Animations& anims, const std::string name)
{
  for (const auto& [spritesheet, sprites] : anims.animations) {
    const auto s =
      std::find_if(sprites.begin(), sprites.end(), [&name](const SpriteAnimation& spr) { return spr.name == name; });
    if (s != std::end(sprites))
      return { spritesheet, *s };
  }

  std::println("CONFIG ERROR: sprite not found: {}", name);
  exit(1); // explode!
}

void
set_sprite(entt::registry& r, const entt::entity& e, const std::string& sprite)
{
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto [spritesheet, anim] = find_animation(anims, sprite);

  auto& sprc = r.get<SpriteComponent>(e);
  sprc.tex_pos.x = anim.animation_frames[0].x;
  sprc.tex_pos.y = anim.animation_frames[0].y;
};

void
set_sprite_custom(entt::registry& r, const entt::entity& e, const std::string& sprite, int tex_unit)
{
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto [spritesheet, anim] = find_animation(anims, sprite);
  // const auto& tag = r.get<TagComponent>(e);

  auto& sc = r.get<SpriteComponent>(e);
  // sc.colour = get_lin_colour_by_tag(r, tag);
  sc.tex_unit = tex_unit;
  sc.total_sx = spritesheet.nx;
  sc.total_sy = spritesheet.ny;
  sc.tex_pos.x = anim.animation_frames[0].x;
  sc.tex_pos.y = anim.animation_frames[0].y;
  sc.tex_pos.w = anim.animation_frames[0].w;
  sc.tex_pos.h = anim.animation_frames[0].h;
};

} // namespace game2d
