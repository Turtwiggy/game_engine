#version 330 core

layout(location = 0) out vec4 out_color;

in vec2 v_uv;

uniform sampler2D tex;

void
main()
{
  // color = sprite_colour; //* texture(tex, TexCoords);
  // color = sprite_colour * texture(tex, TexCoords);
  // color = vec4(1.0, 0.0, 1.0, 1.0);
  out_color = texture(tex, v_uv);
}