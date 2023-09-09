#version 110

attribute vec4 vertex; // <vec2 position, vec2 texCoords>

varying vec2 v_tex;

uniform mat4 projection;
uniform mat4 model;

void
main()
{
  v_tex = vertex.zw;
  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}