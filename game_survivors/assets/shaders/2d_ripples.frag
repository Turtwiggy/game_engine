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

// this key is replaced by the engine with
// e.g. "uniform sampler2D tex_monochrome_transparent_packed"
// {{ generate_user_samplers }}
// uniform int RENDERER_TEX_UNIT_COUNT;

vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec3 fade(vec3 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec2 P){
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
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
  float rot = fs_in.v_sprite_global_pos_and_rot.z;


  // float something = length(col.rgb);
  // out_colour.rgb = vec3(sign(something));
  // out_colour.a = something > 0 ? 1.0 : 0.0;

  vec2 uv_for_1_pixel = vec2(1.0) / fs_in.v_sprite_size;

  //  make the line uv-idependent (so its like 5px)
  float min_val = 0.62;
  float max_val = min_val + uv_for_1_pixel.y * 3;

  if(v_uv.y < min_val){
    out_colour.rgb = vec3(0.0f);
    return;
  }



  // draw the white line
  if(v_uv.y > min_val && v_uv.y < max_val)
  {
    out_colour.rgb = vec3(0.3, 0.6, 0.9);
    out_colour.a = 1.0f;
    return;
  }
  
  // idea: make the bottom half of the sprite blur
  // vec2 offset_tex = v_uv + vec2(-time/10.0, time/10.0);
  // float noise = cnoise(offset_tex * 10.0) * 0.1;
  
  out_colour.rgb = vec3(0.3, 0.5, 0.9);
  // out_colour.rgb = vec3(0.3, 0.5, 0.9)* vec3(noise, noise, noise);
  out_colour.a = 1.0f;

  // vec2 doubled_uv = vec2(v_uv.x, (v_uv.y - 0.5) * 2.0f) - vec2(0.5f);
  // float hmm = length(doubled_uv) - 0.5;
  // if(hmm > 0.0)
  //   return;
  
  // out_colour.rg = doubled_uv;
  // out_colour.rgb = vec3(0.5f);
  // out_colour.r = hmm;

  // vec3 col = vec4(v_uv,0.5+0.5*sin(time),1.0).xyz;
  // vec2 center = vec2(0.5);
  // float speed = 0.35;
  // float x = doubled_uv.x;
  // float y = doubled_uv.y;
  // float r = -(x*x + y*y);
	// float z = 1.0 + 0.5*sin((r+time*speed)/0.013);
  // out_colour.rgb = col * vec3(z,z,z);

  // out_colour.b = v_uv.y;
  // out_colour.a = 0.6f;
}