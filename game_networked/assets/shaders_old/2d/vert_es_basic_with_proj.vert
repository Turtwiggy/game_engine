#version 110

// gl2.0 es 110 changes:
// in = attribute
// out = varying
// completely remove layout()

attribute vec4 vertex;
attribute mat4 model;

uniform mat4 projection;

void
main()
{
  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}