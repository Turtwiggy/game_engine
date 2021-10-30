#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 sprite_pos;
layout(location = 3) in float tex_index;
layout(location = 4) in mat4 model;

out vec2 v_tex;
out vec4 v_colour;
out vec2 v_sprite_pos;
out float v_tex_index;

uniform mat4 projection;

void
main()
{
  // FragPos = vec3(model * vec4(vertex.xy, 0.0, 1.0));
  v_tex = vertex.zw;
  v_colour = colour;
  v_sprite_pos = sprite_pos;
  v_tex_index = tex_index;

  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}