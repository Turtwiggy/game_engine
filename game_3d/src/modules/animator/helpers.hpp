#pragma once

#include "modules/animator/components.hpp"
#include "modules/models/components.hpp"

namespace game2d {

void
load_animations(SINGLE_AnimatorComponent& anims, SINGLE_ModelsComponent& models);

void
play_animation(SINGLE_AnimatorComponent& anims, Animation* a);
void
update_animation(SINGLE_AnimatorComponent& anims, float dt);

} // namespace game2d