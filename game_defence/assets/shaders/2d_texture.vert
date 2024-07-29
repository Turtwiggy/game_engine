#version 150


layout(location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 v_uv;

uniform mat4 projection;
uniform mat4 model;

void
main()
{
  v_uv = vertex.zw;
  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}