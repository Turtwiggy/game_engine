#version 330 core
out vec4 out_colour;

uniform vec4 colour;

void
main()
{
  out_colour = colour;
}