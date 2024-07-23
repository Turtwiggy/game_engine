#version 300 es
precision highp float;

layout(location = 0) in vec4 out_colour;

uniform vec4 colour;

void
main()
{
  out_colour = colour;
}