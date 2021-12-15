// header
#include "ai.hpp"

// c++ headers
#include "math.h"

namespace game2d {

void
enemy_ai::move_along_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s)
{
  dir = glm::normalize(dir);
  obj.velocity.x = dir.x * obj.speed_current;
  obj.velocity.y = dir.y * obj.speed_current;
};

void
enemy_ai::enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  glm::vec2 ab = player.pos - obj.pos;
  move_along_vector(obj, ab, delta_time_s);
};

void
enemy_ai::enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
{
  // calculate a vector ab
  glm::vec2 ab = player.pos - obj.pos;
  // calculate the point halfway between ab
  glm::vec2 half_point = glm::vec2(obj.pos) + (ab / 2.0f);
  // calculate the vector at a right angle
  glm::vec2 normal = glm::vec2(-ab.y, ab.x);

  // expensive(?) distance calc
  float distance = glm::distance(glm::vec2(obj.pos), glm::vec2(player.pos));
  float half_distance = distance / 2.0f;

  // offset the midpoint via normal
  float amplitude = half_distance * sin(glm::radians(obj.approach_theta_degrees));
  half_point += (glm::normalize(normal) * amplitude);
  glm::vec2 dir = glm::normalize(half_point - glm::vec2(obj.pos));
  move_along_vector(obj, dir, delta_time_s);
};

} // namespace game2d