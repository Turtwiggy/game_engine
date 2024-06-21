#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

in VS_OUT
{
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
}
fs_in;

uniform vec3 object_colour;
uniform vec3 light_colour;

void
main()
{
  FragColor = vec4(light_colour * object_colour, 1.0);
}
