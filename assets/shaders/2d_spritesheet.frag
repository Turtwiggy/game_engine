#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D tex;
uniform vec4 sprite_colour;

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
    TexCoords.x / num_cols + desired_x * scale_x,
    TexCoords.y / num_rows + desired_y * scale_y
  );
  // clang-format on

  color = sprite_colour * texture(tex, sprite_tex_coord);
}