// version prepended to file when loaded by engine.
// #version 130

layout(location = 0) in vec4 out_colour;

uniform vec4 colour;

void
main()
{
  out_colour = colour;
}