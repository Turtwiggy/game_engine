#version 330 core

layout(location = 0) out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;

void
main()
{
  // out_colour = vec4(v_uv.x, v_uv.y, 0.0, 1.0);
  out_colour = vec4(1.0, 0.0, 0.0, 1.0);
}
