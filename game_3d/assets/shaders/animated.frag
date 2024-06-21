#version 460 core

out vec4 colour;

in vec2 o_uv;
in vec4 o_colour;

void
main()
{
  colour = o_colour;

  colour = vec4(1.0, 1.0, 1.0, 1.0);
}