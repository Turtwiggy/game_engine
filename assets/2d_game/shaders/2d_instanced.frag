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

void
main()
{
  // clang-format off
  vec2 sprite_tex_coord = vec2(
    v_tex.x / num_cols + v_sprite_pos.x * scale_x,
    v_tex.y / num_rows + v_sprite_pos.y * scale_y
  );
  // clang-format on

  vec4 c;

  if (v_sprite_pos.x == 0 && v_sprite_pos.y == 0) {
    c = v_colour * texture(tex, v_tex);
  } else {
    c = v_colour * texture(tex, sprite_tex_coord);
  }

  out_colour = c;
}