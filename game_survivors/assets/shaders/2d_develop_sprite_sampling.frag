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

layout(std140) uniform Data {
  mat4 projection_zoomed;
	mat4 view;
	vec4[32] light_positions;
  vec2 camera_pos;
  vec2 screenshake;
  float time;
  float zoom;
  float tilesize;
};


// https://www.shadertoy.com/view/MllBWf
// https://www.shadertoy.com/view/ltBfRD
// https://www.shadertoy.com/view/MlB3D3
// https://jorenjoestar.github.io/post/pixel_art_filtering/

uniform sampler2D tex;
uniform vec2 screen_wh;

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  vec2 iResolution = screen_wh; 		 // e.g. 640, 360
  vec2 tex_size =  textureSize(tex, 0);

  // const float size   =    1.00; // fat pixel size
  // const float zoom   =    1.00; // upper zoom range
  const float radius =   64.00; // scroll radius
  const float speed  =    0.10; // speed
  float mtime = time * speed;
  // float scale = size + (cos(mtime) + 1.0) * (zoom - 1.0) * size * 0.5;
	vec2 offset = vec2(cos(mtime), sin(mtime)) * radius;
  // vec2 pos = vec2(v_uv.x, 1.0 - v_uv.y) * iResolution;
  // vec2 pix = (pos + offset) / scale;
  // pix /= tex_size;
  // pix.y = 1.0 - pix.y;

  // vec2 sprite_uv = vec2(
  //   (v_sprite_wh.x * v_uv.x) / v_sprite_max.x + v_sprite_pos.x * (1.0f/v_sprite_max.x),
  //   (v_sprite_wh.y * v_uv.y) / v_sprite_max.y + v_sprite_pos.y * (1.0f/v_sprite_max.y)
  // );

  vec2 sprite_pos = vec2(0, 0);
  vec2 sprite_wh = vec2(48, 22);
  vec2 sprite_max = vec2(48, 22);
  vec2 sprite_pixels = tex_size / sprite_max;
  // vec2 sprite_pos = v_sprite_pos;
  // vec2 sprite_wh = v_sprite_wh;
  // vec2 sprite_max = v_sprite_max;
  // vec2 sprite_uv = (v_sprite_pos + v_sprite_wh * v_uv) / v_sprite_max;

  // note: out-colour.rgb is linear
  // out_colour.rgb = texture(tex, sprite_uv).rgb;
  // out_colour.a = texture(tex, fat_uv).a;

  // vec2 uv = ((v_uv * tex_size) + offset); // should be imgui window size, but meh
  vec2 uv = (v_uv * tex_size);

  // uv.x += 0.5;
  uv += offset;
  // uv = floor(uv) + 0.5;

  if (v_uv.x < 0.33)
  {
    uv = floor(uv) + .5;
  }
  else if(v_uv.x < 0.66)
  {
    uv = floor(uv) + min(fract(uv) / fwidth(uv), 1.0) - 0.5;
  }
  else
  {
    vec2 seam = floor(uv + 0.5);
    vec2 dudv = fwidth(uv);
    uv = seam + clamp( (uv - seam) / dudv, -0.5, 0.5);
  }

  uv /= tex_size;

  vec3 rgb = texture(tex, uv).rgb;
  float a = texture(tex, uv).a;
  if(a > 0.0f && a < 1.0f){
    out_colour.rgb = rgb;
    // out_colour.rgb = vec3(0.9, 0.3, 0.3);
    out_colour.a = a;
    out_colour.a = 1.0f;
    return;
  }

  out_colour.rgb = rgb;
  out_colour.a = a;

  // out_colour.r = out_colour.a;
  // out_colour.a = 1.0f;
}
