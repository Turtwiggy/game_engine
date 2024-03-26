#version 460

layout(location = 0) in vec4 vertex; // xy and uv
layout(location = 1) in vec4 colour;
layout(location = 2) in vec4 sprite_pos;
layout(location = 3) in vec4 sprite_width_and_max;
layout(location = 4) in float tex_unit;
layout(location = 5) in mat4 model;

out vec2 v_uv;
out vec4 v_colour;
out vec2 v_sprite_pos;
out vec2 v_sprite_wh;
out vec2 v_sprite_max;
out float v_tex_unit;

uniform mat4 view;
uniform mat4 projection;

void
main()
{
  // FragPos = vec3(model * vec4(vertex.xy, 0.0, 1.0));
  v_uv = vertex.zw;
  v_colour = colour;
  v_sprite_pos = sprite_pos.xy;
  v_sprite_wh = sprite_width_and_max.xy;
  v_sprite_max = sprite_width_and_max.zw;
  v_tex_unit = tex_unit;

  gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
}