// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform sampler2D tex_monochrome_transparent_packed;
uniform sampler2D tex_gameicons;
uniform sampler2D tex_blueberry_dark;
uniform sampler2D tex_goblin_brawl_background;

uniform vec2 viewport_wh;
uniform int RENDERER_TEX_UNIT_COUNT;

#define _SUPERSAMPLING_2X2_RGSS

// https://discussions.unity.com/t/how-to-keep-sprites-sharp-and-crisp-even-while-rotating-solved/737314/6
vec4 tex2dss(sampler2D tex, vec2 uv, float bias, float aascale)
{

    vec4 col = vec4(0.0);

    // get uv derivatives
    vec2 dx = dFdx(uv);
    vec2 dy = dFdy(uv);
  
#if defined(_SUPERSAMPLING_2X2_RGSS)
    // MSAA style "four rooks" rotated grid super sampling
    // samples the texture 4 times

    vec2 uvOffsets = vec2(0.125, 0.375);

    col += texture(tex, uv + uvOffsets.x * dx + uvOffsets.y * dy, bias);
    col += texture(tex, uv - uvOffsets.x * dx - uvOffsets.y * dy, bias);
    col += texture(tex, uv + uvOffsets.y * dx - uvOffsets.x * dy, bias);
    col += texture(tex, uv - uvOffsets.y * dx + uvOffsets.x * dy, bias);

    col *= 0.25;

#elif defined(_SUPERSAMPLING_8X_HALTON)
    // 8 points from a 2, 3 Halton sequence
    // similar to what TAA uses, though they usually use more points
    // samples the texture 8 times
    // better quality for really fine details

    float2 halton[8] = {
        float2(1,-3) / 16.0,
        float2(-1,3) / 16.0,
        float2(5,1) / 16.0,
        float2(-3,-5) / 16.0,
        float2(-5,5) / 16.0,
        float2(-7,-1) / 16.0,
        float2(3,7) / 16.0,
        float2(7,-7) / 16.0
    };

    for (int i=0; i<8; i++)
      col += tex2Dbias(tex, vec4(uv + halton[i].x * dx + halton[i].y * dy, 0, bias));

    col *= 0.125;

#elif defined(_SUPERSAMPLING_16X16_OGSS)
    // brute force ground truth 16x16 ordered grid super sampling
    // samples the texture 256 times! you should not use this!
    // does not use tex2Dbias, but instead always samples the top mip

    float gridDim = 16;
    float halfGridDim = gridDim / 2;

    for (float u=0; u<gridDim; u++)
    {
        float uOffset = (u - halfGridDim + 0.5) / gridDim;
        for (float v=0; v<gridDim; v++)
        {
            float vOffset = (v - halfGridDim + 0.5) / gridDim;
            col += tex2Dlod(tex, vec4(uv + uOffset * dx + vOffset * dy, 0, 0));
        }
    }

    col /= (gridDim * gridDim);
#else
  // no super sampling, just bias
  // col = tex2Dbias(tex, vec4(uv, 0, bias));
#endif
    return col;
}

void
main()
{
  int index = int(v_tex_unit);

  out_colour = v_colour;

  // v_uv goes from 0 to 1
  // convert from 0 to 1 to the width/height desired 
  vec2 sprite_uv = vec2(
    (v_sprite_wh.x * v_uv.x) / v_sprite_max.x + v_sprite_pos.x * (1.0f/v_sprite_max.x),
    (v_sprite_wh.y * v_uv.y) / v_sprite_max.y + v_sprite_pos.y * (1.0f/v_sprite_max.y)
  );

  if(index == RENDERER_TEX_UNIT_COUNT){
    float bias = -0.75;
    float aa_scale = 1.25;

    float outline_size = 2.0f;
    vec2 texel_size = vec2(1.0 / float(viewport_wh.x), 1.0 / float(viewport_wh.y));

    // vec4 spritesheet_col = tex2dss(tex_monochrome_transparent_packed, sprite_uv, bias, aa_scale);
    vec4 spritesheet_col = texture(tex_monochrome_transparent_packed, sprite_uv);

    // float l_pix = tex2dss(tex_monochrome_transparent_packed, sprite_uv + vec2(-texel_size.x, 0), bias, aa_scale).a;
    // float u_pix = tex2dss(tex_monochrome_transparent_packed, sprite_uv + vec2(0, texel_size.y), bias, aa_scale).a;
    // float r_pix = tex2dss(tex_monochrome_transparent_packed, sprite_uv + vec2(texel_size.x, 0), bias, aa_scale).a;
    // float b_pix = tex2dss(tex_monochrome_transparent_packed, sprite_uv + vec2(0, -texel_size.y), bias, aa_scale).a;
    float l_pix = texture(tex_monochrome_transparent_packed, sprite_uv + vec2(-texel_size.x, 0)).a;
    float r_pix = texture(tex_monochrome_transparent_packed, sprite_uv + vec2(texel_size.x, 0)).a;
    float u_pix = texture(tex_monochrome_transparent_packed, sprite_uv + vec2(0, texel_size.y)).a;
    float d_pix = texture(tex_monochrome_transparent_packed, sprite_uv + vec2(0, -texel_size.y)).a;

    out_colour *= spritesheet_col;
    
    // float inline = (1.0f - l_pix * u_pix * r_pix * d_pix) * spritesheet_col.a;
    float outline = max(max(l_pix, u_pix), max(r_pix, d_pix)) - spritesheet_col.a;

    // NOTE: outline_col should be linear, not srgb
    vec4 outline_col = vec4(1.0, 1.0, 1.0, 1.0);
    // out_colour = mix(out_colour, outline_col, outline);
    out_colour = outline == 1.0f ? outline_col : vec4(0.0, 0.0, 0.0, 1.0);
    return;
  }

  out_colour = vec4(0.0, 0.0, 0.0, 1.0);
}