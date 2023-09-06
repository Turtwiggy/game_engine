#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

// constants
#define PI 3.141596
#define TAU 6.2831853071795864769252867665590
#define MAX_STEPS 64
#define RAYS_PER_PIXEL 64
#define EPSILON 0.001

// uniforms
uniform sampler2D u_distance_data;
uniform sampler2D u_noise;
uniform sampler2D u_basetexture;
uniform vec2 screen_wh;

bool RANGE(float v, float lo, float hi) { return (v - hi) * (v - lo) > 0.0; }
bool surfacemarch(vec2 pix, vec2 dir, float noise, out vec2 hitpos, out vec3 hitcol) {
   float aspect = screen_wh.x / screen_wh.y;
   vec2 pixel = vec2(pix.x * aspect, pix.y);

   for(float ray = 0.0, dst = 0.0, i = 0.0; i < MAX_STEPS; i += 1.0) {
      vec2 raypos = pixel + (dir * ray);
      raypos.x /= aspect;
      ray += (dst = texture2D(u_distance_data, raypos).r);

      if (RANGE(raypos.x, 0.0, 1.0) || RANGE(raypos.y, 0.0, 1.0)) return false;
      if (dst <= EPSILON) {

         // Random sample either surface emitters or previous frame emission pixel.
         vec3 srceCol = texture2D(u_basetexture, raypos).rgb;
         float srceValue = max(srceCol.r, max(srceCol.g, srceCol.b));
         vec2 srcePos = raypos;

         raypos *= aspect;
         raypos -= (dir * ray * noise);
         raypos /= aspect;
               
         vec3 destCol = texture2D(u_basetexture, raypos).rgb;
         float destValue = max(destCol.r, max(destCol.g, destCol.b));
         vec2 destPos = raypos;

         float check = step(destValue, srceValue);
         hitcol = (check > 0.0)? srceCol : mix(destCol, srceCol, 0.5);
         hitpos = (check > 0.0)? srcePos : destPos;
         return true;
      }
   }
   return false;
}

vec3 tonemap(vec3 color, float dist) {
    // INVERSE SQR LAW FOR LIGHT: (not my preferred, visually)
    //return color * (1.0 / (1.0 + dot(dist / min(screen_wh.x, screen_wh.y))));
    
    // LINEAR DROP OFF:
    return color * (1.0 - (dist / min(screen_wh.x, screen_wh.y)));
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
   vec3  colors = vec3(0.0);
   float emissv = 0.0,
   gnoise = texture2D(u_noise, v_uv).r,
   gangle = gnoise * TAU;

   const float RAY_DELTA = TAU * (1.0/RAYS_PER_PIXEL);
   for(float i = 0.0; i < TAU; i += RAY_DELTA) {
      vec2 hitpos = v_uv; vec3 hitcol = vec3(0.0);
      surfacemarch(v_uv, vec2(cos(gangle + i), -sin(gangle + i)), gnoise, hitpos, hitcol);
      hitcol = tonemap(hitcol, distance(v_uv * screen_wh, hitpos * screen_wh));
      emissv += max(hitcol.r, max(hitcol.g, hitcol.b));
      colors += hitcol;
   }

   vec3 color = (colors / emissv) * (emissv / RAYS_PER_PIXEL);
   out_color = vec4(lin_to_srgb(color), 1.0);
}