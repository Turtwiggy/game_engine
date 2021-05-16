#version 330 core
out vec4 out_colour;

in vec4 v_colour;

void
main()
{
  out_colour = v_colour;
  // out_colour = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}