#version 330 core
out vec4 out_colour;

in vec2 v_tex;
in vec4 v_colour;
in vec2 v_sprite_pos;

// hard coded for kennynl spritesheet atm
const int num_cols = 48;
const int num_rows = 22;
const float scale_x = 1.0f / num_cols;
const float scale_y = 1.0f / num_rows;

uniform sampler2D tex;
// uniform bool aces_tone_mapping;

vec3
aces(vec3 x)
{
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float
aces(float x)
{
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void
main()
{
  // clang-format off
  vec2 sprite_tex_coord = vec2(
    v_tex.x / num_cols + v_sprite_pos.x * scale_x,
    v_tex.y / num_rows + v_sprite_pos.y * scale_y
  );
  // clang-format on

  vec4 c = v_colour * texture(tex, sprite_tex_coord);
  // if (aces_tone_mapping) {
  // out_colour = vec4(aces(c.xyz), c.w);
  // } else {
  out_colour = c;
  // }
}