#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D tex;
uniform vec4 sprite_colour;

uniform int num_cols;
uniform float time_per_sprite;
uniform float age;

void
main()
{
  // doing this for every frag is pretty wasteful
  int col = int(age / time_per_sprite);

  float scale_x = 1.0 / num_cols;
  float scale_y = 1.0 / 22.0;

  // clang-format off
  // vec2 sprite_tex_coord = vec2(
  //   TexCoords.x / num_cols + col * (1.0 / num_cols), 
  //   TexCoords.y / 22.0 + 0 * (1.0 / 22.0)
  // );

  vec2 sprite_tex_coord = vec2(
    TexCoords.x / num_cols + 0 * (scale_x),
    TexCoords.y / 22.0f + 1 * scale_y
  );

  // clang-format on

  color = sprite_colour * texture(tex, sprite_tex_coord);
}