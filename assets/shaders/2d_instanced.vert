#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 colour;
layout(location = 3) in vec4 worldspace_pos;

out vec2 v_tex;
out vec4 v_colour;

void
main()
{
  v_tex = vertex.zw;
  v_colour = colour;

  // gl_Position = vec4(vertex.xy, 0.0, 1.0);
  gl_Position = vec4(worldspace_pos.xy, 0.0, 1.0);
}