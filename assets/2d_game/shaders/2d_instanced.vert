#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 sprite_pos;
layout(location = 3) in mat4 model;

out vec2 v_tex;
out vec4 v_colour;
out vec2 v_sprite_pos;

uniform mat4 projection;
uniform bool shake;
uniform float time;

void
main()
{
  v_tex = vertex.zw;
  v_colour = colour;
  v_sprite_pos = sprite_pos;

  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);

  if (shake) {
    float strength = 0.01;
    gl_Position.x += cos(time * 10) * strength;
    gl_Position.y += cos(time * 15) * strength;
  }
}