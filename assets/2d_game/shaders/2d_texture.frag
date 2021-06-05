#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D tex;
uniform vec4 sprite_colour;

void
main()
{
  // color = sprite_colour; //* texture(tex, TexCoords);
  color = sprite_colour * texture(tex, TexCoords);
  // color = vec4(1.0, 0.0, 0.0, 1.0);
}