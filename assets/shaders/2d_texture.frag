#version 330 core
out vec4 out_color;

in vec2 v_tex;

uniform sampler2D textures[1];

void
main()
{
  // color = sprite_colour; //* texture(tex, TexCoords);
  // color = sprite_colour * texture(tex, TexCoords);
  // color = vec4(1.0, 0.0, 1.0, 1.0);
  out_color = texture(textures[0], v_tex);
}