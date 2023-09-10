#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;
uniform float u_dist_mod = 1.0;
uniform vec2 screen_wh;

float V2_F16(vec2 v) { return v.x + (v.y / 255.0); }
vec2 F16_V2(float f) { return vec2(floor(f * 255.0) / 255.0, fract(f * 255.0)); }

void
main()
{
  vec4 jfuv = texture2D(tex, v_uv);
  vec2 jumpflood = vec2(V2_F16(jfuv.rg),V2_F16(jfuv.ba));
  out_color = vec4(F16_V2(distance(v_uv, jumpflood)), 0.0, 1.0);
}