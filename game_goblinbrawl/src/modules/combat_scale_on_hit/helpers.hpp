#pragma once

namespace game2d {

float
flip(float x);

float
ease_in(float t);

float
ease_out(float t);

float
ease_in_out(float t);

// spike is a lerp that returns to it's original position
float
spike(float t);

} // namespace game2d