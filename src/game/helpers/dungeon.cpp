#include "dungeon.hpp"

#include "modules/physics/components.hpp"

#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

const int GRID_SIZE = 16;

void
create_room(entt::registry& r, Dungeon& d, int offset_x, int offset_y, int width, int height)
{
  int x1 = offset_x;
  int y1 = offset_y;
  int x2 = x1 + width;
  int y2 = y1 + height;

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {

      ENTITY_TYPE et = ENTITY_TYPE::FLOOR;

      if (x == 0)
        et = ENTITY_TYPE::WALL;
      if (y == 0)
        et = ENTITY_TYPE::WALL;
      if (x == width - 1)
        et = ENTITY_TYPE::WALL;
      if (y == height - 1)
        et = ENTITY_TYPE::WALL;

      entt::entity e = create_gameplay(r, et);
      SpriteComponent s = create_sprite(r, e, et);
      TransformComponent t = create_transform(r, e);

      glm::ivec2 grid_index = { offset_x + x, offset_y + y };
      glm::ivec2 grid_position = engine::grid::grid_space_to_world_space(grid_index, GRID_SIZE);
      t.position = { grid_position.x, grid_position.y, 0 };

      r.emplace<SpriteComponent>(e, s);
      r.emplace<TransformComponent>(e, t);

      // add entity as only entity in grid position
      std::pair<int, int> tilemap_index = { grid_index.x, grid_index.y };
      if (d.tilemap.contains(tilemap_index)) {
        std::vector<entt::entity>& entities = d.tilemap[tilemap_index];
        for (int j = 0; j < entities.size(); j++)
          r.destroy(entities[j]);
        entities.clear();
        entities.push_back(e);
      } else {
        d.tilemap[tilemap_index] = { e };
      }

      //
    }
  }
};

// inspired by:
// https://github.com/scikit-image/scikit-image/blob/main/skimage/draw/_draw.pyx
void
create_line(int r0, int c0, int r1, int c1, std::vector<std::pair<int, int>>& results)
{
  int r = r0;
  int c = c0;
  int dr = glm::abs(r1 - r0);
  int dc = glm::abs(c1 - c0);
  int sc = c1 - c > 0 ? 1 : -1;
  int sr = r1 - r > 0 ? 1 : -1;

  int steep = 0;
  if (dr > dc) {
    steep = 1;
    std::swap(c, r);
    std::swap(dc, dr);
    std::swap(sc, sr);
  }
  int d = (2 * dr) - dc;

  std::pair<int, int> res;

  for (int i = 0; i < dc; i++) {
    if (steep) {
      res.first = c;
      res.second = r;
      results.push_back(res);
    } else {
      res.first = r;
      res.second = c;
      results.push_back(res);
    }
    while (d >= 0) {
      r = r + sr;
      d = d - (2 * dc);
    }
    c = c + sc;
    d = d + (2 * dr);
  }

  // res.first = r1;
  // res.second = c1;
  // results.push_back(res);
};

void
create_walls(entt::registry& r, Dungeon& d, std::vector<std::pair<int, int>>& coords)
{
  for (int i = 0; i < coords.size(); i++) {
    int x = coords[i].first;
    int y = coords[i].second;

    ENTITY_TYPE et = ENTITY_TYPE::FLOOR;
    entt::entity e = create_gameplay(r, et);
    SpriteComponent s = create_sprite(r, e, et);
    TransformComponent t = create_transform(r, e);

    glm::ivec2 pos = engine::grid::grid_space_to_world_space({ x, y }, GRID_SIZE);
    t.position = { pos.x, pos.y, 0 };

    r.emplace<SpriteComponent>(e, s);
    r.emplace<TransformComponent>(e, t);

    // add entity as only entity in grid position
    std::pair<int, int> tilemap_index = { x, y };
    if (d.tilemap.contains(tilemap_index)) {
      std::vector<entt::entity>& entities = d.tilemap[tilemap_index];
      for (int j = 0; j < entities.size(); j++)
        r.destroy(entities[j]);
      entities.clear();
      entities.push_back(e);
    } else {
      d.tilemap[tilemap_index] = { e };
    }
  }
}

// Create an L-shaped tunnel between two points
void
create_tunnel(entt::registry& r, Dungeon& d, int x1, int y1, int x2, int y2)
{
  int corner_x = 0;
  int corner_y = 0;

  // move horisontally, then vertically
  corner_x = x2;
  corner_y = y1;

  // move vertically, then horizontally
  // corner_x = x1;
  // corner_y = y2;

  // generate coordinates based on bresenham line agoorithm

  // a) x1, y1 to corner_x, corner_y
  std::vector<std::pair<int, int>> line_0;
  create_line(x1, y1, corner_x, corner_y, line_0);
  create_walls(r, d, line_0);

  // b) corner_x, corner_y to x2, y2
  std::vector<std::pair<int, int>> line_1;
  create_line(corner_x, corner_y, x2, y2, line_1);
  create_walls(r, d, line_1);
};

void
generate_dungeon(entt::registry& r, Dungeon& d)
{
  for (auto const& [key, val] : d.tilemap) {
    for (int i = 0; i < (val).size(); i++) {
      r.destroy(val[i]);
    }
  }
  d.tilemap.clear();

  // generate square rooms
  create_room(r, d, 20, 15, 10, 15);
  create_room(r, d, 35, 15, 10, 15);

  glm::ivec2 room_1_center = { (20 + 20 + 10) / 2, (15 + 15 + 15) / 2 };
  glm::ivec2 room_2_center = { (35 + 35 + 10) / 2, (15 + 15 + 15) / 2 };
  create_tunnel(r, d, room_1_center.x, room_1_center.y, room_2_center.x, room_2_center.y);
};

} // namespace game2d