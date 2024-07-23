#version 300 es
precision highp float;

layout(location = 0) in vec4 vertex; // xy and uv
layout(location = 1) in vec4 colour;
layout(location = 2) in vec4 sprite_pos;
layout(location = 3) in vec4 sprite_width_and_max;
layout(location = 4) in float tex_unit;
layout(location = 5) in mat4 model;

out vec2 v_uv;

uniform mat4 view;
uniform mat4 projection;

void
main()
{
  v_uv = vertex.zw;
  // v_colour = colour;
  // v_sprite_pos = sprite_pos.xy;
  // v_sprite_wh = sprite_width_and_max.xy;
  // v_sprite_max = sprite_width_and_max.zw;
  // v_tex_unit = tex_unit;
  // v_vertex = vec4( model * vec4(vertex.xy, 1.0, 1.0)).xy;

  gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
}