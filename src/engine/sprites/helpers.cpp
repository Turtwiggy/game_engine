// header
#include "engine/sprites/helpers.hpp"

#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"

// other libs
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

// std libs
#include <SDL2/SDL_log.h>
#include <format>
#include <fstream>
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
load_sprites(SINGLE_Animations& anims, const std::string& path)
{
#if defined(_RELEASE)
  SDL_Log("%s", std::format("loading sprite config: {}", path).c_str());
#endif
  std::ifstream f(path);

  // if there's an error here,
  // as it's the first thing loaded,
  // it probably means: assets haven't been provided for the build
  const json data = json::parse(f);

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
  const auto info = std::make_pair(spritesheet, sprites);
  anims.animations.push_back(std::move(info));
};

std::pair<Spritesheet, SpriteAnimation>
find_animation(const SINGLE_Animations& anims, const std::string& name)
{
  for (const auto& [spritesheet, sprites] : anims.animations) {
    const auto s =
      std::find_if(sprites.begin(), sprites.end(), [&name](const SpriteAnimation& spr) { return spr.name == name; });
    if (s != std::end(sprites))
      return { spritesheet, *s };
  }

  SDL_Log("%s", std::format("CONFIG ERROR: sprite not found: {}", name).c_str());
  exit(1); // explode!
}

void
set_sprite(entt::registry& r, const entt::entity e, const std::string& sprite)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto [spritesheet, anim] = find_animation(anims, sprite);

  auto& sc = r.get<SpriteComponent>(e);
  sc.tex_unit = search_for_texture_unit_by_spritesheet_path(ri, spritesheet.path)->unit;
  sc.total_sx = spritesheet.nx;
  sc.total_sy = spritesheet.ny;

  sc.tex_pos.x = anim.animation_frames[0].x;
  sc.tex_pos.y = anim.animation_frames[0].y;
  sc.tex_pos.w = anim.animation_frames[0].w;
  sc.tex_pos.h = anim.animation_frames[0].h;
  if (anim.angle_degrees != 0.0f)
    sc.angle_radians = glm::radians(anim.angle_degrees);
};

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite)
{
  // convert to imgui representation
  const auto [ss, frames] = find_animation(get_first_component<SINGLE_Animations>(r), sprite);
  const int size_x = ss.px_total;
  const int size_y = ss.py_total;
  const int cols_x = ss.nx;
  const int cols_y = ss.ny;
  const float pixels_x = size_x / float(cols_x);
  const float pixels_y = size_y / float(cols_y);
  const glm::ivec2 offset = { frames.animation_frames[0].x, frames.animation_frames[0].y };
  const ImVec2 tl = ImVec2(((offset.x * pixels_x + 0.0f) / size_x), ((offset.y * pixels_y + 0.0f) / size_y));
  const ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / size_x), ((offset.y * pixels_y + pixels_y) / size_y));
  return { tl, br };
};

} // namespace game2d
