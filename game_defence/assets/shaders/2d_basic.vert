// version prepended to file when loaded by engine.
// #version 130

layout(location = 0) in vec4 pos;

void
main()
{
  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}