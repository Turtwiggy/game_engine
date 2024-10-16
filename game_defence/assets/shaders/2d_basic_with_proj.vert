// version prepended to file when loaded by engine.

// gl2.0 es changes:
// in = attribute
// out = varying
// completely remove layout()

layout(location = 0) in vec4 vertex;
layout(location = 1) in mat4 model;

uniform mat4 view;
uniform mat4 projection;

void
main()
{
  gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
}