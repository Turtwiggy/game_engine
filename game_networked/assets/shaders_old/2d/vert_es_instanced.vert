#version 110

attribute vec4 vertex;
attribute vec4 colour;
attribute vec4 sprite_pos_and_spritesheet;
attribute float tex_unit;
attribute mat4 model;

varying vec2 v_tex;
varying vec4 v_colour;
varying vec2 v_sprite_pos;
varying vec2 v_sprites;
varying float v_tex_unit;

uniform mat4 view;
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

  gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);

  // translational and rotational screenshake
  // gl_Position.x *= cos(time * 10) * strength; 
  // gl_Position.y *= cos(time * 10) * strength; 
  // gl_Position.z *= cos(time * 10) * strength; 
}