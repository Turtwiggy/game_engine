#version 330

layout(location = 0) out vec4 out_colour;

uniform vec4 colour;

void
main()
{
  out_colour = colour;
}