// header
#include "sprites/helpers.hpp"

// other libs
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// std libs
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace game2d {

struct Frame
{
  int x = 0;
  int y = 0;
};

void
load_sprites(std::vector<SpriteAnimation>& sprites, const std::string path)
{
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

    for (auto& frame : frames)
      anim.animation_frames.push_back({ frame["x"], frame["y"] });

    if (el.contains("angle"))
      anim.angle_degrees = el["angle"];

    sprites.push_back(anim);
  }
};

SpriteAnimation
find_animation(const std::vector<SpriteAnimation>& sprites, const std::string name)
{
  auto s = std::find_if(sprites.begin(), sprites.end(), [&name](const SpriteAnimation& spr) { return spr.name == name; });
  if (s != std::end(sprites))
    return *s;
  else {
    std::cerr << "CONFIG ERROR: sprite not found: " << name << "\n";
    exit(1); // explode!
  }
}

} // namespace game2d
