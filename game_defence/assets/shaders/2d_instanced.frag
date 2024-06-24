#version 460

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform sampler2D tex_kenny;
uniform sampler2D tex_gameicons;
uniform sampler2D tex_unit_space_background;
uniform sampler2D tex_unit_mainmenu_background;
uniform sampler2D tex_unit_spacestation_0;
uniform sampler2D tex_unit_studio_logo;
uniform float screen_w;
uniform float screen_h;
uniform int RENDERER_TEX_UNIT_COUNT;

// fat pixel approach
// vec2
// uv_to_fatpixel(vec2 uv){
//   float tp = float(float(screen_h) / float(screen_w));
//   vec2 tex_size = vec2(screen_w, screen_h);
//   vec2 pixel = v_uv * tex_size;
//   vec2 fat_pixel = floor(pixel) + 0.5;
//   fat_pixel += 1 - clamp((1.0 - fract(pixel)) * tp, 0, 1); // subpixel aa algorithm
//   return fat_pixel / tex_size;
// }


vec2 clamp_uv(vec2 uv){
  vec2 tex_size = vec2(screen_w / 1000.0, screen_h / 1000.0);
  vec2 pixel = v_uv * tex_size;
  vec2 fat_pixel = floor(pixel) + 0.5;
  return fat_pixel;
};

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

    vec2 sprite_uv_tmp = clamp_uv(v_uv);

    out_colour = v_colour;

    // index set on cpu side...
    // WARNING: seems bad

    // starting at 5, as the renderer makes use of 4 textures...

    if(index == RENDERER_TEX_UNIT_COUNT){
      out_colour *= texture(tex_kenny, sprite_uv);
    }
    else if(index == (RENDERER_TEX_UNIT_COUNT+1)){
      out_colour *= texture(tex_gameicons, sprite_uv);
      return; // texture uses 0, 0
    }
    else if(index == RENDERER_TEX_UNIT_COUNT+2){
      out_colour *= texture(tex_unit_space_background, sprite_uv);
      return; // texture uses 0, 0
    }
    else if(index == RENDERER_TEX_UNIT_COUNT+3){
      out_colour *= texture(tex_unit_mainmenu_background, sprite_uv);
      return; // texture uses 0, 0
    }
    else if(index == RENDERER_TEX_UNIT_COUNT+4){
      out_colour *= texture(tex_unit_spacestation_0, sprite_uv);
      return; // texture uses 0, 0
    }
     else if(index == RENDERER_TEX_UNIT_COUNT+5){
      out_colour *= texture(tex_unit_studio_logo, sprite_uv);
      return; // texture uses 0, 0
    }

  }

  // Sample texture directly
  if ((v_sprite_pos.x == 0.0f && v_sprite_pos.y == 0.0f)) { // a whole texture
    out_colour = v_colour;
    return;
  }
}


// heartbeast approach?
// https://www.youtube.com/watch?v=2JbhkZe22bE&list=RDCMUCrHQNOyU1q6BFEfkNq2CYMA&index=25
// vec2 uv = v_tex_unit;
// vec2 size = vec2(textureSize(textures[1], 0));
// vec2 pixel = vec2(1.0) / size;
// uv -= pixel * vec2(0.5);
// vec2 uv_pixels = uv * size;
// vec2 delta_pixel = fract(uv_pixels) - vec2(0.5);
// vec2 ddxy = fwidth(uv_pixels);
// vec2 mip = log2(ddxy) - 0.5;
// vec4 tex_main =
//   textureLod(textures[1], uv + (clamp(delta_pixel / ddxy, 0.0, 1.0) - delta_pixel) * pixel, min(mip.x, mip.y));

// fat pixel approach
// vec2
// uv_to_fatpixel(vec2 uv){
//   float tp = float(float(screen_h) / float(screen_w));
//   vec2 tex_size = vec2(screen_w, screen_h);
//   vec2 pixel = v_uv * tex_size;
//   vec2 fat_pixel = floor(pixel) + 0.5;
//   fat_pixel += 1 - clamp((1.0 - fract(pixel)) * tp, 0, 1); // subpixel aa algorithm
//   return fat_pixel / tex_size;
// }

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
