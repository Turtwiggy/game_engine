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

uniform sampler2D tex_to_outline;
uniform sampler2D tex_heightmap;
uniform vec2 screen_wh;

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

float scale(float value, float inMin, float inMax, float outMin, float outMax) {
  float clampedValue = clamp(value, inMin, inMax);
  float normalized = (clampedValue - inMin) / (inMax - inMin);
  return outMin + normalized * (outMax - outMin);
}

// https://www.shadertoy.com/view/XdXGW8
vec2 grad( ivec2 z )  // replace this anything that returns a random vector
{
    // 2D to 1D  (feel free to replace by some other)
    int n = z.x+z.y*11111;

    // Hugo Elias hash (feel free to replace by another one)
    n = (n<<13)^n;
    n = (n*(n*n*15731+789221)+1376312589)>>16;

#if 0
    // simple random vectors
    return vec2(cos(float(n)),sin(float(n)));
    
#else
    // Perlin style vectors
    n &= 7;
    vec2 gr = vec2(n&1,n>>1)*2.0-1.0;
    return ( n>=6 ) ? vec2(0.0,gr.x) : 
           ( n>=4 ) ? vec2(gr.x,0.0) :
                              gr;
#endif                              
}
float noise( in vec2 p )
{
    ivec2 i = ivec2(floor( p ));
     vec2 f =       fract( p );
	
	vec2 u = f*f*(3.0-2.0*f); // feel free to replace by a quintic smoothstep instead

    return mix( mix( dot( grad( i+ivec2(0,0) ), f-vec2(0.0,0.0) ), 
                     dot( grad( i+ivec2(1,0) ), f-vec2(1.0,0.0) ), u.x),
                mix( dot( grad( i+ivec2(0,1) ), f-vec2(0.0,1.0) ), 
                     dot( grad( i+ivec2(1,1) ), f-vec2(1.0,1.0) ), u.x), u.y);
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

  vec3 tex_col = texture(tex_to_outline, v_uv).rgb;
  vec3 tex_heightmap_col = texture(tex_heightmap, v_uv).rgb;
  // out_colour.rgb = tex_heightmap_col;
  out_colour.rgb = vec3(0.0f);
  out_colour.a = 1.0f;

  float island_cutoff = 0.69;
  float outline_cutoff = 0.65;

  /*
    // float distance_fade = 1.0 - (1.0f / 4.0f);
  float lu = 1.0 - max(l_max, r_max);
  float lr = 1.0 - max(u_max, d_max);
  // float lc = 1.0 - max(l_max, u_max);

  float is_outline = max(max(l_max, u_max), max(r_max, d_max)) - c_max > 0.0 ? 1.0 : 0.0;
  float is_near_outline = max(max(l_max, u_max), max(r_max, d_max)) - c_max > 0.0 ? 1.0 : 0.0;

  // Additional samples for gradient
  vec3 far_l_col = texture(tex_to_outline, v_uv - vec2(texel_size.x * 3.0, 0)).rgb;
  vec3 far_r_col = texture(tex_to_outline, v_uv + vec2(texel_size.x * 3.0, 0)).rgb;
  vec3 far_u_col = texture(tex_to_outline, v_uv + vec2(0, texel_size.y * 31.0)).rgb;
  vec3 far_d_col = texture(tex_to_outline, v_uv - vec2(0, texel_size.y * 31.0)).rgb;
  float far_l_max = max(far_l_col.r, max(far_l_col.g, far_l_col.b)) > 0.0 ? 1.0 : 0.0;
  float far_r_max = max(far_r_col.r, max(far_r_col.g, far_r_col.b)) > 0.0 ? 1.0 : 0.0;
  float far_u_max = max(far_u_col.r, max(far_u_col.g, far_u_col.b)) > 0.0 ? 1.0 : 0.0;
  float far_d_max = max(far_d_col.r, max(far_d_col.g, far_d_col.b)) > 0.0 ? 1.0 : 0.0;
  float is_far_outline = max(max(far_l_max, far_u_max), max(far_r_max, far_d_max)) - c_max > 0.0 ? 1.0 : 0.0;
  
  // Color logic: near = white, far = grey
    vec3 outline_color = vec3(0.0);
    
    if (is_near_outline > 0.0) {
        outline_color = vec3(1.0, 1.0, 1.0); // White for near outline
    } else if (is_far_outline > 0.0) {
        outline_color = vec3(0.6, 1.0, 0.2); // Grey for far outline
    }

  // Combine for gradient effect
  // float outline_intensity = is_outline * 1.0 + is_far_outline * 0.3;
  // outline_intensity = min(outline_intensity, 1.0);

  if((lu) <= 0.0f)
  out_colour = vec4(lr);
  else 
  out_colour = vec4(1.0, 0.0, 0.0, 1.0);
  return;
  */

  // if(tex_heightmap_col.r >= island_cutoff){
  //   // out_colour.rgb = vec3(1.0); // "island" colour
  // }
  // else 

  float heightmap_info = 0.0f;
  if(tex_heightmap_col.r >= outline_cutoff){
    // "foam" depth mask. colour 
    // here is where the foam should show up.

    // scale [0.65, 0.69] to [0, 1];
    float percent = scale(tex_heightmap_col.r, outline_cutoff, island_cutoff, 0, 1);

    // screenspace to worldspace coords.
    vec2 uv = v_uv - 0.5;
    uv *= zoom;
    vec2 cam_uv = camera_pos + (uv * screen_wh);
	  cam_uv /= screen_wh;
    vec2 final_uv = cam_uv;
    
    // convert to scrolling uv
    final_uv -= time*0.01f;

    float f = noise(32 * final_uv);
    f = 0.5 + 0.5*f;
    heightmap_info = f * percent;

    out_colour.rgb = vec3(heightmap_info);
    return;
  }

  float ripple_freq = 1.0f;
  float ripple_ampl = 0.005f;
  float ripple = 0.0001 + sin(time * ripple_freq) * ripple_ampl;
  vec2 ripple_offset = vec2(ripple);

  // vec2 sprite_uv = (v_uv - v_pos)
  vec2 texel_size = 3.0 / vec2(textureSize(tex_to_outline, 0));
  vec2 up = vec2(0, texel_size.y + ripple);
  vec2 rgt = vec2(texel_size.x + ripple, 0);
  vec2 down = vec2(0, -texel_size.y - ripple);
  vec2 left = vec2(-texel_size.x - ripple, 0);

  vec3 col   = texture(tex_to_outline, v_uv).rgb;
  vec3 l_col = texture(tex_to_outline, v_uv + left).rgb; 
  vec3 r_col = texture(tex_to_outline, v_uv + rgt).rgb;
  vec3 u_col = texture(tex_to_outline, v_uv + up).rgb;
  vec3 d_col = texture(tex_to_outline, v_uv + down).rgb; 

  float c_max = max(col.r, max(col.g, col.b)) > 0.0 ? 1.0 : 0.0;
  float l_max = max(l_col.r, max(l_col.g, l_col.b)) > 0.0 ? 1.0 : 0.0;
  float r_max = max(r_col.r, max(r_col.g, r_col.b)) > 0.0 ? 1.0 : 0.0;
  float u_max = max(u_col.r, max(u_col.g, u_col.b)) > 0.0 ? 1.0 : 0.0;
  float d_max = max(d_col.r, max(d_col.g, d_col.b)) > 0.0 ? 1.0 : 0.0;

  // float sprite_inline = (1.0f - l_col * u_col * r_col * d_col) * col.a;
  float hmm = max(max(l_max, u_max), max(r_max, d_max)) - c_max;
  float sprite_outline = hmm  > 0.0 ? 1.0 : 0.0;

  // out_colour.rgb =  out_colour.rgb;
  out_colour = sprite_outline * vec4(1.0f);
}