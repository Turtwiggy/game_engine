// header
#include "modules/sprites/helpers.hpp"

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
  std::cout << "loading sprite config: " << path << std::endl;
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
    SpriteAnimation s;
    s.name = el["name"];

    auto frames = el["frames"];
    for (auto& frame : frames) {
      Frame f;
      f.x = frame["x"];
      f.y = frame["y"];
      s.animation_frames.push_back({ f.x, f.y });
    }

    if (el.contains("angle")) {
      s.animation_angle_degrees = el["angle"];
    }

    sprites.push_back(s);
  }
};

SpriteAnimation
find_animation(const std::vector<SpriteAnimation>& sprites, const std::string name)
{
  auto s =
    std::find_if(sprites.begin(), sprites.end(), [&name](const SpriteAnimation& spr) { return spr.name == name; });
  if (s != std::end(sprites))
    return *s;
  else {
    std::cerr << "sprite not found: " << name << std::endl;
    exit(1); // explode!
  }
}

} // namespace game2d

// std::vector<sprite::type>
// convert_int_to_sprites(int damage)
// {
//   std::vector<sprite::type> numbers;

//   if (damage == 0) {
//     numbers.push_back(sprite::type::NUMBER_0);
//     return numbers;
//   }

//   int number = damage;
//   // this iterates over number from right to left.
//   // e.g. 1230 will iterate as 0, 3, 2, 1
//   while (number > 0) {
//     int digit = number % 10;
//     number /= 10;
//     if (digit == 9)
//       numbers.push_back(sprite::type::NUMBER_9);
//     if (digit == 8)
//       numbers.push_back(sprite::type::NUMBER_8);
//     if (digit == 7)
//       numbers.push_back(sprite::type::NUMBER_7);
//     if (digit == 6)
//       numbers.push_back(sprite::type::NUMBER_6);
//     if (digit == 5)
//       numbers.push_back(sprite::type::NUMBER_5);
//     if (digit == 4)
//       numbers.push_back(sprite::type::NUMBER_4);
//     if (digit == 3)
//       numbers.push_back(sprite::type::NUMBER_3);
//     if (digit == 2)
//       numbers.push_back(sprite::type::NUMBER_2);
//     if (digit == 1)
//       numbers.push_back(sprite::type::NUMBER_1);
//     if (digit == 0)
//       numbers.push_back(sprite::type::NUMBER_0);
//   }

//   // now reverse numbers in to e.g. 1230
//   std::reverse(numbers.begin(), numbers.end());
//   return numbers;
// }