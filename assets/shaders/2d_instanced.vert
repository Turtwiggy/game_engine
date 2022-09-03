#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec4 sprite_pos_and_spritesheet;
layout(location = 3) in float tex_unit;
layout(location = 4) in mat4 model;

out vec2 v_tex;
out vec4 v_colour;
out vec2 v_sprite_pos;
out vec2 v_sprites;
out float v_tex_unit;

uniform mat4 projection;

uniform float screenshake_trauma;

void
main()
{
  // FragPos = vec3(model * vec4(vertex.xy, 0.0, 1.0));
  v_tex = vertex.zw;
  v_colour = colour;
  v_sprite_pos = sprite_pos_and_spritesheet.xy;
  v_sprites = sprite_pos_and_spritesheet.zw;
  v_tex_unit = tex_unit;

  gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);

  // translational and rotational screenshake
  // gl_Position.x *= cos(time * 10) * strength; 
  // gl_Position.y *= cos(time * 10) * strength; 
  // gl_Position.z *= cos(time * 10) * strength; 
}