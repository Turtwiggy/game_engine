#include "line.hpp"

#include "maths/maths.hpp"
#include "renderer/components.hpp"

namespace game2d {

LineInfo
generate_line(const glm::ivec2& a, const glm::ivec2& b, const int width)
{
  glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
  glm::vec2 nrm_dir = raw_dir;
  if (raw_dir.x != 0.0f || raw_dir.y != 0.0f)
    nrm_dir = glm::normalize(raw_dir);
  float angle = engine::dir_to_angle_radians(nrm_dir);

  LineInfo line;
  line.position = { (a.x + a.x + raw_dir.x) / 2, (a.y + a.y + raw_dir.y) / 2 };
  line.rotation = angle + engine::HALF_PI;
  line.scale.x = width;
  line.scale.y = sqrt(pow(raw_dir.x, 2) + pow(raw_dir.y, 2));
  return line;
};

// void
// create_line(int r0, int c0, int r1, int c1, std::vector<std::pair<int, int>>& results)
// {
//   int r = r0;
//   int c = c0;
//   int dr = glm::abs(r1 - r0);
//   int dc = glm::abs(c1 - c0);
//   int sc = c1 - c > 0 ? 1 : -1;
//   int sr = r1 - r > 0 ? 1 : -1;

//   int steep = 0;
//   if (dr > dc) {
//     steep = 1;
//     std::swap(c, r);
//     std::swap(dc, dr);
//     std::swap(sc, sr);
//   }
//   int d = (2 * dr) - dc;

//   std::pair<int, int> res;

//   for (int i = 0; i < dc; i++) {
//     if (steep) {
//       res.first = c;
//       res.second = r;
//       results.push_back(res);
//     } else {
//       res.first = r;
//       res.second = c;
//       results.push_back(res);
//     }
//     while (d >= 0) {
//       r = r + sr;
//       d = d - (2 * dc);
//     }
//     c = c + sc;
//     d = d + (2 * dr);
//   }

//   // res.first = r1;
//   // res.second = c1;
//   // results.push_back(res);
// };

} // namespace game2d