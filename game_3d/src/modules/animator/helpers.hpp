#pragma once

#include "modules/animator/components.hpp"
#include "modules/models/components.hpp"

namespace game2d {

Animation
load_animation(const std::string& path, const Model& model);
void
load_animations(SINGLE_AnimatorComponent& anims);

void
update_animation(SINGLE_AnimatorComponent& anims, float dt);

} // namespace game2d