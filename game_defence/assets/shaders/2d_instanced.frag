// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform sampler2D tex_kenny;
uniform sampler2D tex_gameicons;
uniform sampler2D tex_unit_spacestation_0;
uniform sampler2D tex_unit_studio_logo;
uniform sampler2D tex_unit_custom;
uniform vec2 viewport_wh;
uniform int RENDERER_TEX_UNIT_COUNT;

// vec2 clamp_uv(vec2 uv){
//   vec2 tex_size = vec2(viewport_wh.x / 1000.0, viewport_wh.y / 1000.0);
//   vec2 pixel = v_uv * tex_size;
//   vec2 fat_pixel = floor(pixel) + 0.5;
//   return fat_pixel;
// }

// heartbeast approach?
// https://www.youtube.com/watch?v=2JbhkZe22bE&list=RDCMUCrHQNOyU1q6BFEfkNq2CYMA&index=25
// vec4 texturePointSmooth(sampler2D tex, vec2 uv, vec2 pixel_size){
  // vec2 size = vec2(textureSize(tex, 0));
  // vec2 pixel = vec2(1.0) / size;
  // uv -= pixel * vec2(0.5);
  // vec2 uv_pixels = uv * size;
  // vec2 delta_pixel = fract(uv_pixels) - vec2(0.5);
  // vec2 ddxy = fwidth(uv_pixels);
  // vec2 mip = log2(ddxy) - 0.5;
  // return textureLod(tex, uv + (clamp(delta_pixel / ddxy, 0.0, 1.0) - delta_pixel) * pixel, min(mip.x, mip.y));
// }

// fat pixel approach
// vec2
// uv_to_fatpixel(vec2 uv){
//   float tp = float(float(viewport_wh.y) / float(viewport_wh.x));
//   vec2 tex_size = viewport_wh;
//   vec2 pixel = v_uv * tex_size;
//   vec2 fat_pixel = floor(pixel) + 0.5;
//   fat_pixel += 1 - clamp((1.0 - fract(pixel)) * tp, 0, 1); // subpixel aa algorithm
//   return fat_pixel / tex_size;
// }

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

  //
}

void
main()
{
  int index = int(v_tex_unit);
  
  // A spritesheet texture
  {
    // v_uv goes from 0 to 1
    // convert from 0 to 1 to the width/height desired 
    vec2 sprite_uv = vec2(
      (v_sprite_wh.x * v_uv.x) / v_sprite_max.x + v_sprite_pos.x * (1.0f/v_sprite_max.x),
      (v_sprite_wh.y * v_uv.y) / v_sprite_max.y + v_sprite_pos.y * (1.0f/v_sprite_max.y)
    );

    out_colour = v_colour;

    // bias, range -4, 4
    float bias = -0.75;
    // aa_scale, range: 0.75, 10
    float aa_scale = 1.25;

    // index set on cpu side...
    // WARNING: seems bad

    if(index == RENDERER_TEX_UNIT_COUNT){
      out_colour *= tex2dss(tex_kenny, sprite_uv, bias, aa_scale);
      // out_colour *= texture(tex_kenny, sprite_uv);
    }
    else if(index == (RENDERER_TEX_UNIT_COUNT+1)){
      out_colour *= texture(tex_gameicons, sprite_uv);
      return; // texture uses 0, 0
    }
    else if(index == RENDERER_TEX_UNIT_COUNT+2){
      out_colour *= texture(tex_unit_spacestation_0, sprite_uv);
      return; // texture uses 0, 0
    }
    else if(index == RENDERER_TEX_UNIT_COUNT+3){
      out_colour *= texture(tex_unit_studio_logo, sprite_uv);
      return; // texture uses 0, 0
    }
    else if(index == RENDERER_TEX_UNIT_COUNT+4){
      out_colour *= texture(tex_unit_custom, sprite_uv);
      return; // texture uses 0, 0
    }

  }

  // Sample texture directly
  if ((v_sprite_pos.x == 0.0f && v_sprite_pos.y == 0.0f)) { // a whole texture
    out_colour = v_colour;
    return;
  }
}




// vec2
// uv_nearest(vec2 uv, ivec2 texture_size)
// {
//   vec2 pixel = uv * texture_size;
//   pixel = floor(pixel) + .5;
//   return pixel / texture_size;
// }

// this approach?
// https://csantosbh.wordpress.com/2014/02/05/automatically-detecting-the-texture-filter-threshold-for-pixelated-magnifications/
// int w = int(screen_w);
// int h = int(screen_h);
// vec2 vUv = v_uv * vec2(w, h);
// vec2 alpha = 0.7 * vec2(dFdx(vUv.x), dFdy(vUv.y));
// vec2 x = fract(vUv);
// vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
// vec2 uv = (floor(vUv) + x_) / vec2(w, h);

// vec2
// uv_cstantos(vec2 uv, vec2 res)
// {
//   vec2 pixels = uv * res;
//   // Updated to the final article
//   vec2 alpha = 0.7 * fwidth(pixels);
//   vec2 pixels_fract = fract(pixels);
//   vec2 pixels_diff = clamp(.5 / alpha * pixels_fract, 0, .5) + clamp(.5 / alpha * (pixels_fract - 1) + .5, 0, .5);
//   pixels = floor(pixels) + pixels_diff;
//   return pixels / res;
// }
