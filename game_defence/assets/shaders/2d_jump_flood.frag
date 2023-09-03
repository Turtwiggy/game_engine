#version 330 core

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;

void
main()
{
  vec4 scene_col = texture(tex, v_uv);
  
  out_color = scene_col;
}