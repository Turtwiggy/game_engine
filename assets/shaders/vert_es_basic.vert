#version 110

attribute vec4 pos;

void
main()
{
  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}