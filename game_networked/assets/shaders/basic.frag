#version 460 core

out vec4 colour;

in vec2 uv;

uniform vec3 u_colour;

void
main()
{
  colour = vec4(u_colour, 1.0);
}