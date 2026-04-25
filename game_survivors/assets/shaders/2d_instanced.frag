// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_size; // e.g. 16, 16
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  vec3 v_sprite_global_pos_and_rot;
  float v_tex_unit;
} fs_in;

// this key is replaced by the engine with
// e.g. "uniform sampler2D tex_monochrome_transparent_packed"
{{ generate_user_samplers }}
uniform int RENDERER_TEX_UNIT_COUNT;
uniform vec2 screen_wh;

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

float
linear_to_srgb(float f)
{
  if (f <= 0.0031308f)
    return 12.92f * f;
  return 1.055f * pow(f, 1.0f / 2.4f) - 0.055f;
}

vec3 lin_to_srgb(vec3 color)
{
  vec3 x = color.rgb * 12.92;
  vec3 y = 1.055 * pow(clamp(color.rgb, 0.0, 1.0), vec3(0.4166667)) - 0.055;
  vec3 clr = color.rgb;
  clr.r = (color.r < 0.0031308) ? x.r : y.r;
  clr.g = (color.g < 0.0031308) ? x.g : y.g;
  clr.b = (color.b < 0.0031308) ? x.b : y.b;
  return clr.rgb;
}

vec2 uv_aa_smoothstep( vec2 uv, vec2 res, float width )
{
  uv = uv * res;
  vec2 uv_floor = floor(uv + 0.5);
  vec2 uv_fract = fract(uv + 0.5);
  vec2 uv_aa = fwidth(uv) * width * 0.5;
  uv_fract = smoothstep(
      vec2(0.5) - uv_aa,
      vec2(0.5) + uv_aa,
      uv_fract
      );

  return (uv_floor + uv_fract - 0.5) / res;
}

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  // out_colour.a = 1.0f;

  // special case: render the "empty" sprite as just the colour
  if(index == RENDERER_TEX_UNIT_COUNT && (v_sprite_pos.x == 0.0f && v_sprite_pos.y == 0.0f)) {
    out_colour = v_colour;
    out_colour.a = 1.0f;
    return;
  }

  // A spritesheet texture
  {
    // v_uv goes from 0 to 1
    // convert from 0 to 1 to the width/height desired
    vec2 tex_size = vec2(0, 0);
    {{ generate_tex_size }}
    vec2 sprite_pos = v_sprite_pos;
    vec2 sprite_wh = v_sprite_wh;
    vec2 sprite_max = v_sprite_max;
    vec2 sprite_pixels = tex_size / sprite_max; // e.g. (16, 16)
    vec2 sprite_size = fs_in.v_sprite_size;

    vec2 sprite_uv = (v_sprite_pos + v_sprite_wh * v_uv) / v_sprite_max;
    vec2 offset = fs_in.v_sprite_global_pos_and_rot.xy;
    
    // try keep the pixel art crisp
    // Try to correct for sprite shimmering
    // float width = 1.5;
    // vec2 pixels = sprite_wh;
    // vec2 raw_uv = uv_aa_smoothstep(sprite_uv, vec2(16, 16), width);
    
    vec2 uv = sprite_uv * tex_size;

    // this is nearest neighbour sampling
    // uv = floor(uv) + .5;
    uv = floor(uv) + min(fract(offset) / fwidth(offset), 1.0) - 0.5; 

    // vec2 seam = floor(uv + 0.5);
    // vec2 dudv = fwidth(offset);
    // uv = seam + clamp( (uv - seam) / dudv, -0.5, 0.5);

    // uv = uv_aa_smoothstep(uv, sprite_size, 1.5);

    uv /= tex_size;

    // the engine generates code here e.g. 
    vec2 tex_uv = uv;
    vec4 col = vec4(1.0f);
    {{ generate_sampler_if_statements }}

    // out_colour.rgb = lin_to_srgb(srgb_to_lin(v_colour.rgb) * srgb_to_lin(col.rgb));
    out_colour.rgb = v_colour.rgb * col.rgb;
    out_colour.a = col.a;

    // vec4 col = texture(u_textures[index - RENDERER_TEX_UNIT_COUNT], uv).rgba;
    // out_colour.rgb = v_colour.rgb * col.rgb;
    // out_colour.a = col.a;
  }
}