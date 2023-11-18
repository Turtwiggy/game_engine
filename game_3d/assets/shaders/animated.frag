#version 460 core

out vec4 colour;

in vec2 o_uv;
in vec4 o_colour;

void
main()
{
  colour = o_colour;

  colour = vec4(0.7, 0.0, 0.7, 1.0);
}