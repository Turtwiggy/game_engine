#version 150


out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos;
in vec2 v_sprite_wh;
in vec2 v_sprite_max;
in float v_tex_unit;

uniform sampler2D tex_jflood;
uniform sampler2D tex_emitters_and_occluders;
uniform vec2 screen_wh;

float V2_F16(vec2 v) { return v.x + (v.y / 255.0); }
vec2 F16_V2(float f) { return vec2(floor(f * 255.0) / 255.0, fract(f * 255.0)); }

void
main()
{
  vec4 jfuv = texture(tex_jflood, v_uv);
  vec2 jumpflood = vec2(V2_F16(jfuv.rg),V2_F16(jfuv.ba));
  vec2 dst = F16_V2(distance(v_uv, jumpflood));

  float dst_sign = texture(tex_emitters_and_occluders, v_uv).a > 0.0 ? 0.0 : 1.0;

  out_colour = vec4(dst, dst_sign, 1.0);
}