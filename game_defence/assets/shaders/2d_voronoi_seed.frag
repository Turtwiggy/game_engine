#version 460

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

  // any pixel with .a > 0 will be recognised as an emitter or occluder
  out_color = vec4(v_uv.x * scene_col.a, v_uv.y * scene_col.a, 0.0, 1.0);

  // out_color = vec4(v_uv.x, v_uv.y, 0.0, 1.0);
}