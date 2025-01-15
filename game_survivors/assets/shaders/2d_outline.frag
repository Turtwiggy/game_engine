// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  float v_tex_unit;
  vec2 v_vertex;
} fs_in;

uniform sampler2D tex_to_outline;

// uniform vec2 viewport_wh;
// uniform int RENDERER_TEX_UNIT_COUNT;

float
SRGBFloatToLinearFloat(const float f)
{
  if (f <= 0.04045f)
    return f / 12.92f;
  return pow((f + 0.055f) / 1.055f, 2.4f);
}

vec3 srgb_to_lin(vec3 color)
{
  vec3 result;
  result.x = SRGBFloatToLinearFloat(color.r / 255.0f);
  result.y = SRGBFloatToLinearFloat(color.g / 255.0f);
  result.z = SRGBFloatToLinearFloat(color.b / 255.0f);
  return result;
}

void
main()
{
	vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  // vec2 sprite_uv = (v_uv - v_pos)
  vec2 texel_size = 2.0 / vec2(textureSize(tex_to_outline, 0));
  vec2 up = vec2(0, texel_size.y);
  vec2 rgt = vec2(texel_size.x, 0);

  vec4 col   = texture(tex_to_outline, v_uv);
  vec4 l_col = texture(tex_to_outline, v_uv - rgt);
  vec4 r_col = texture(tex_to_outline, v_uv + rgt);
  vec4 u_col = texture(tex_to_outline, v_uv + up);
  vec4 d_col = texture(tex_to_outline, v_uv - up);

  float col_max = max(col.r, max(col.g, col.b)) > 0.0 ? 1.0 : 0.0;
  float l_max = max(l_col.r, max(l_col.g, l_col.b)) > 0.0 ? 1.0 : 0.0;
  float r_max = max(r_col.r, max(r_col.g, r_col.b)) > 0.0 ? 1.0 : 0.0;
  float u_max = max(u_col.r, max(u_col.g, u_col.b)) > 0.0 ? 1.0 : 0.0;
  float d_max = max(d_col.r, max(d_col.g, d_col.b)) > 0.0 ? 1.0 : 0.0;

  // float inline = (1.0f - l_pix * u_pix * r_pix * d_pix) * col.a;
  float outline = max(max(l_max, u_max), max(r_max, d_max)) - col_max > 0.0 ? 1.0 : 0.0;
  if(outline > 0.0f)
    out_colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}