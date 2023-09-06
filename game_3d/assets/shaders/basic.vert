#version 460 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_uv;

out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void
main()
{
  gl_Position = projection * view * model * vec4(v_pos, 1.0);
  uv = v_uv;
}