#pragma once

namespace game2d {

struct Size
{
  int w, h;

  Size() = default;
  Size(int w, int h)
    : w(w)
    , h(h){};
};

} // namespace game2d