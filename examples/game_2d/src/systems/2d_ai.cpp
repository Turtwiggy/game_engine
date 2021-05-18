// your header
#include "systems/2d_ai.hpp"

// your project headers
#include "engine/maths_core.hpp"

namespace game2d {

void
ai_chase_player(GameObject2D& player, const float delta_time, std::vector<GameObject2D>& enemies)
{
  for (int i = 0; i < enemies.size(); ++i) {
    GameObject2D& e = enemies[i];

    // rotate to player
    glm::vec2 dir = player.pos - e.pos;
    dir = glm::normalize(dir);
    e.angle_radians = atan2(dir.y, dir.x);
    e.angle_radians += fightingengine::PI / 2.0f;

    // move to player
    const float monster_speed = 50.0f;
    e.velocity = glm::vec2(monster_speed);
    float x = glm::sin(e.angle_radians) * e.velocity.x;
    float y = -glm::cos(e.angle_radians) * e.velocity.y;
    e.pos.x += x * delta_time;
    e.pos.y += y * delta_time;
  }
}

} // namespace game2d