#pragma once

// c++ lib headers
#include <vector>

// other lib headers
#include <glm/glm.hpp>

// engine headers
#include "engine/maths_core.hpp"

// game headers
#include "2d_game_object.hpp"
#include "spritemap.hpp"

namespace game2d {

namespace vfx {

// vfx death "splat"
void
spawn_death_splat(engine::RandomState& rnd,
                  const GameObject2D& enemy,
                  const sprite::type s,
                  std::vector<GameObject2D>& ents);

// vfx impact "splats"
void
spawn_impact_splats(engine::RandomState& rnd,
                    const GameObject2D& src,
                    const GameObject2D& dst,
                    const sprite::type sprite,
                    int damage_amount,
                    std::vector<GameObject2D>& ents);

} // namespace vfx

} // namespace game2d