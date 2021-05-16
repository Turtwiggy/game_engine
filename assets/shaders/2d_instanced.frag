#version 330 core
out vec4 out_colour;

in vec2 v_tex;
in vec4 v_colour;

uniform sampler2D tex;

// hard coded for kennynl spritesheet atm
const int num_cols = 48;
const int num_rows = 22;
const float scale_x = 1.0f / num_cols;
const float scale_y = 1.0f / num_rows;

uniform int desired_x;
uniform int desired_y;

void
main()
{
  // clang-format off
  vec2 sprite_tex_coord = vec2(
    v_tex.x / num_cols + desired_x * scale_x,
    v_tex.y / num_rows + desired_y * scale_y
  );
  // clang-format on

  out_colour = v_colour * texture(tex, sprite_tex_coord);
}