#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

// constants
#define PI 3.141596
#define RAYS_PER_PIXEL 32
#define MAX_STEPS 64
#define EPSILON 0.001

// uniforms
uniform sampler2D u_distance_data;
uniform sampler2D u_scene_data;
uniform float u_emission_multi = 1.0;
uniform float u_dist_mod = 1.0;
uniform vec2 screen_wh;
uniform float time;

float random (vec2 st) 
{
   return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

bool raymarch(vec2 origin, vec2 dir, float aspect, out vec2 hit_pos)
{
  float current_dist = 0.0;
  for(int i = 0; i < MAX_STEPS; i++)
  {
    vec2 sample_point = origin + dir * current_dist;

    // when we sample the distance field we need to convert back to uv space.
    sample_point.x /= aspect; 

    // early exit if we hit the edge of the screen.
    if(sample_point.x > 1.0 || sample_point.x < 0.0 || sample_point.y > 1.0 || sample_point.y < 0.0)
        return false;

    float dist_to_surface = texture(u_distance_data, sample_point).r / u_dist_mod;

    // we've hit a surface if distance field returns 0 or close to 0 (due to our distance field using a 16-bit float
    // the precision isn't enough to just check against 0).
    if(dist_to_surface < EPSILON)
    {
        hit_pos = sample_point;
        return true;
    }

    // if we don't hit a surface, continue marching along the ray.
    current_dist += dist_to_surface;
  }
  return false;
}

void get_surface(vec2 uv, out float emissive, out vec3 colour)
{	
   vec4 emissive_data = texture(u_scene_data, uv);
   emissive = max(emissive_data.r, max(emissive_data.g, emissive_data.b)) * u_emission_multi;
   colour = emissive_data.rgb;
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

void
main()
{
  vec3 pixel_col = vec3(0.0);
  float pixel_emis = 0.0;

  vec2 uv = v_uv;
  float aspect = screen_wh.y / screen_wh.x;
  uv.x *= aspect;
  
  float rand2pi = random(v_uv * vec2(time, -time)) * 2.0 * PI;
  // magic number that gives us a good ray distribution.
  float golden_angle = PI * 0.7639320225;

  for(int i = 0; i < RAYS_PER_PIXEL; i++)
  {
    // get our ray dir by taking the random angle and adding golden_angle * ray number.
    float cur_angle = rand2pi + golden_angle * float(i);
    vec2 ray_dir = normalize(vec2(cos(cur_angle), sin(cur_angle)));
    vec2 ray_origin = uv;

   vec2 hit_pos;
   bool hit = raymarch(ray_origin, ray_dir, aspect, hit_pos);
   if(hit)
   {
      float mat_emissive;
      vec3 mat_colour;
      get_surface(hit_pos, mat_emissive, mat_colour);

      pixel_emis += mat_emissive;
      pixel_col += mat_colour;
   }
  }

  pixel_col /= pixel_emis;
  pixel_emis /= float(RAYS_PER_PIXEL);
  
  vec3 res = lin_to_srgb(pixel_emis * pixel_col);
  out_color = vec4(res, 1.0);
}