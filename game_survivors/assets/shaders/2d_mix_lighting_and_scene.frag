// version prepended to file when loaded by engine.
//

out vec4 out_color;

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

uniform sampler2D tex_scene_0;         // linear main
uniform sampler2D tex_unit_water;
uniform sampler2D tex_outline;
uniform sampler2D tex_shine_shells;
uniform vec2 viewport_wh;
uniform bool add_grid;
uniform bool invert_colours;

#define NR_MAX_LIGHTS 32

layout(std140) uniform Data {
  mat4 projection_zoomed;
  mat4 view;
  vec2 camera_pos;
  vec2 screenshake;
  vec4[4] player_positions; // xy: pos, z: angle, w: active
  vec4[NR_MAX_LIGHTS] light_positions; // note: xy: pos, z: angle, w: active
  float time;
  float zoom;
  float tilesize;
};


struct Light
{
    bool enabled;
    vec2 position;
    vec4 colour;
    float luminance;

    // not yet used
    // float range;
    // float radius;
};
#define MAX_LIGHTS 32
uniform Light lights[MAX_LIGHTS];

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

// sdf

float circleDist(vec2 p, float radius)
{
    return length(p) - radius;
}

float sdGrid(in vec2 position, in float margin) {

    // Calculate per-axis distance from 0.5 to position mod 1
    vec2 gridDist = abs(fract(position) - 0.5) - margin;

    // Calculate length for round outer corners, by Inigo Quilez
    float outsideDist = length(max(gridDist, 0.0));
    // Calculate inside separately, by Inigo Quilez
    float insideDist = min(max(gridDist.x, gridDist.y), 0.0);

    return outsideDist + insideDist;
}

// https://www.shadertoy.com/view/3ltSW2
float sdCircle( vec2 p, float radius )
{
    return length(p) - radius;
}

// https://www.shadertoy.com/view/7sdXz2
// s = side length
// r = corner radius
float sdRoundSquare( in vec2 p, in float s, in float r )
{
    vec2 q = abs(p)-s+r;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r;
}

float cro(in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }
float dot2( in vec2 v ) { return dot(v,v); }
float cro2( in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }

float sdWedge( in vec2 q, in vec2 a, in vec2 c, in vec2 b )
{
    q -= c, a -= c, b -= c;

    return 
    // distance
    sqrt(min( dot2(q-a*clamp(dot(q,a)/dot(a,a),0.0,1.0)),
              dot2(q-b*clamp(dot(q,b)/dot(b,b),0.0,1.0)) )) * 
    // sign                                  
    sign(max( cro2(a,q),
              cro2(q,b) ));
}

// c is the sin/cos of the angle. r is the radius
float sdPie( in vec2 p, in vec2 c, in float r )
{
  p.x = abs(p.x);
  float l = length(p) - r;
	float m = length(p - c*clamp(dot(p,c),0.0,r) );
  return max(l,m*sign(c.y*p.x-c.x*p.y));
}

// https://www.shadertoy.com/view/4lcBWn
float sdUnevenCapsule( in vec2 p, in vec2 pa, in vec2 pb, in float ra, in float rb )
{
  p  -= pa;
  pb -= pa;
  float h = dot(pb,pb);
  vec2  q = vec2( dot(p,vec2(pb.y,-pb.x)), dot(p,pb) )/h;
  
  //-----------
  
  q.x = abs(q.x);
  
  float b = ra-rb;
  vec2  c = vec2(sqrt(h-b*b),b);
  
  float k = cro(c,q);
  float m = dot(c,q);
  float n = dot(q,q);
  
      if( k < 0.0 ) return sqrt(h*(n            )) - ra;
  else if( k > c.x ) return sqrt(h*(n+1.0-2.0*q.y)) - rb;
                    return m                       - ra;
}

float sdParabola( in vec2 pos, in float k )
{
  pos.x = abs(pos.x);
  
  float ik = 1.0/k;
  float p = ik*(pos.y - 0.5*ik)/3.0;
  float q = 0.25*ik*ik*pos.x;
  
  float h = q*q - p*p*p;
  float r = sqrt(abs(h));

  float x = (h>0.0) ? 
      // 1 root
      pow(q+r,1.0/3.0) + pow(abs(q-r),1.0/3.0)*sign(p) :
      // 3 roots
      2.0*cos(atan(r,q)/3.0)*sqrt(p);
  
  float d = length(pos-vec2(x,k*x*x));
  
  return (pos.x<x)? -d : d;
}

// signed distance to a 2D quad
float sdQuad( in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2, in vec2 p3 )
{
	vec2 e0 = p1 - p0; vec2 v0 = p - p0;
	vec2 e1 = p2 - p1; vec2 v1 = p - p1;
	vec2 e2 = p3 - p2; vec2 v2 = p - p2;
	vec2 e3 = p0 - p3; vec2 v3 = p - p3;

	vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
	vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
	vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
    vec2 pq3 = v3 - e3*clamp( dot(v3,e3)/dot(e3,e3), 0.0, 1.0 );
    
    vec2 ds = min( min( vec2( dot( pq0, pq0 ), v0.x*e0.y-v0.y*e0.x ),
                        vec2( dot( pq1, pq1 ), v1.x*e1.y-v1.y*e1.x )),
                   min( vec2( dot( pq2, pq2 ), v2.x*e2.y-v2.y*e2.x ),
                        vec2( dot( pq3, pq3 ), v3.x*e3.y-v3.y*e3.x ) ));

    float d = sqrt(ds.x);

	return (ds.y>0.0) ? -d : d;
}

// https://www.shadertoy.com/view/lt3BW2
float opSmoothUnion( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

vec3 Tonemap_ACES(const vec3 x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec2 angle_to_dir(float angle){
  return vec2(cos(angle), sin(angle));
}

void main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  vec2 v_vertex = fs_in.v_vertex;
  int index = int(fs_in.v_tex_unit);

  out_color.a = 1.0f;

  // fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis
  // iResolution : is a vec2 with an X value of 640 and a Y value of 360
  vec2 fragCoord = (v_uv * viewport_wh);
  vec2 iResolution = viewport_wh;
  vec2 half_wh = viewport_wh * 0.5;
	vec2 screen_min = camera_pos - half_wh; // e.g. -960, -540 for 1920x1080
	vec2 screen_max = camera_pos + half_wh; // e.g. 960, 540 for 1920x1080

  // sdf grid
  vec3 grid_col = vec3(0.0f);
  if(add_grid) {

    float aspect_y = viewport_wh.y / viewport_wh.x;
    float grid_size = 50.0;

    // shift uv to [-0.5, 0.5] to add uvs surrouding the camera position
    vec2 uv = v_uv - 0.5;
    uv *= zoom;

    // the camera position moves, acting as uv that increases/decreases
    vec2 camera_uv_screen = camera_pos + (uv * viewport_wh);
    camera_uv_screen /= viewport_wh;

    vec2 grid_uv = camera_uv_screen;
    vec2 grid_p = ( viewport_wh / grid_size ) * grid_uv;

    // if the gridsize gets too small and the gridwidth isnt large enough,
    // the grid appears to dissapear. the value 0.05 seems to work until gridsize<10
    float grid_width = 0.02;
    float margin = 0.5;
    if(abs(sdGrid(grid_p, margin)) >= grid_width)
        grid_col = vec3(0.0);  // background
    else
        grid_col = vec3(0.04); // line
  }


  //
  // SDF for lights?
  //
  vec3 lighting_col = vec3(1.0);
  {
		float aspect_x = viewport_wh.x / viewport_wh.y;

		// convert uv to -1 and 1
		// the -1 is to invert the sdf
		vec2 uv = -1 * (2.0 * v_uv - 1.0);
		uv.x *= aspect_x;
		uv *= zoom;

    float d = 1e10;

    for(int i = 0; i < 32; i++){

      // pos.z being 0 indicicates player inactive
      if(light_positions[i].w < 1.0f){
        continue;
      }

      vec2 ppos = vec2(light_positions[i].x, light_positions[i].y );
      float angle = light_positions[i].z;

      // convert worldspace to between -1 and 1.
      vec2 ss = (((ppos - screen_min)/viewport_wh) * 2.0) - 1.0;
      ss.x *= aspect_x;

      vec2 p = uv + ss;
      float PI = 3.14159265359;
      float degrees_to_rad = PI / 180.0;
      float d0 = 0.0f;

      // circle
      // -10 is < [-pi, pi], meaning it cant be an angle
      if(angle == -10.0f)
      {
        float size = 3.0 * 50.0;
        d0 = sdCircle(p, size / (aspect_x * 100));
        d0 = clamp(d0, -1.0, 1.0); // inside distances only
      }

      // flashlight lighting with wedge sdf
      // https://www.shadertoy.com/view/wldXWB
      else 
      {
        // vec2 a = 1.0 * -angle_to_dir(angle + 60 * degrees_to_rad );
        // vec2 b = vec2(0.0, 0.0); // 0, 0 is the worldspace pos
        // vec2 c = 1.0 * -angle_to_dir(angle - 60 * degrees_to_rad );
        // d0 = sdWedge(p, a, b, c);
        // not interested in super far away distances
        // if(d0 <= -1.0)
        //   continue;

        vec2 a = vec2(0.0, 0.0); // 0, 0 is the worldspace pos 
        vec2 b = 0.6 * -angle_to_dir(angle);
        float ra = 0.25;
        float rb = 0.6;
        d0 = sdUnevenCapsule(p, a, b, ra, rb);

        // Calculate the direction vector for the player's angle
        // vec2 a = angle_to_dir(angle);
        // float t = 3.14 * time * (0.5 + 0.5 * cos(3.14 * 0));
        // d0 = sdPie(p,vec2(sin(a.x),cos(a.y)), 1.0);
        // if(d0 < -1)
        //   d0 = 1.0 - abs(d0);
        // d0 = clamp(d0, -1.0, 1.0); // inside distances only

        // float pk = 8.0f; // width
        // float d0 = sdParabola(p, pk);
        // d0 = clamp(d0, -1.0, 1.0); // inside distances only

        // vec2 a = 1.0 * -angle_to_dir(angle + 30 * degrees_to_rad );
        // vec2 b = vec2(0.0, 0.0); // 0, 0 is the worldspace pos
        // vec2 c = 1.0 * -angle_to_dir(angle - 30 * degrees_to_rad );
        // vec2 v1 = a;
        // vec2 v2 = b;
        // vec2 v3 = b;
        // vec2 v4 = c;
        // d0 = sdQuad( p, v1, v2, v3, v4 );
        // d0 = clamp(d0, -1.0, 1.0); // inside distances only
      }

      // smooth it in
      float dt = opSmoothUnion(d, d0, 0.1);
      d = min(d, dt);
      // d = d0;
      // break;
    }

    // coloring
    vec3 no_light_col = vec3(0.0,0.0,0.0);
    vec3 light_col = vec3(255/255.0f, 255/255.0f, 255/255.0f); // 3100k
    vec3 col = (d>0.0) ? no_light_col : light_col;
    col *= 1.0 - exp(-6.0*abs(d));
    // col *= 0.8 + 0.2*cos(128.0*abs(d));
    col = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.015,abs(d)) );

    // vec3 col = (d>0.0) ? vec3(1.0,1.0, 1.0) : vec3(1.0,1.0,1.0);
    // col *= 1.0 - exp(-20.0*abs(d));

    // light falloff outside the "light"
    // col *= exp(-6.0*abs(d));

    // no falloff inside the "light"
    // col = d < 0.0 ? vec3(1.0, 1.0, 1.0) : col;

    // col *= 0.8 + 0.2*cos(150.0*d);
    // col = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.01,abs(d)) );

    // lighting_col.r = 1.0;
    // lighting_col.rgb = vec3(d);
    lighting_col.rgb = col;

    // if no lights (d == 1e10), set vec3 to 1.0 to display full scene
    lighting_col = d == 1e10 ? vec3(1.0): lighting_col;
  }

  vec4 scene_lin = texture(tex_scene_0, v_uv);
  vec4 outline_col = texture(tex_outline, v_uv);
  vec3 srgb_water = texture(tex_unit_water, v_uv).rgb;

  if (length(scene_lin.rgb) > 0.0) {
    out_color.rgb = lin_to_srgb( lighting_col * scene_lin.rgb );
  } else {
    out_color.rgb = lighting_col * srgb_water;
  } 
  // out_color.rgb = lighting_col;

  if(outline_col.r > 0.0f)
      out_color.rgb = vec3(1.0, 0.0, 0.0);

  out_color.rgb += grid_col;

  // vignette
  // vec2 vig_uv = fragCoord.xy / iResolution.xy;
  // vig_uv *=  1.0 - vig_uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
  // float vig = vig_uv.x*vig_uv.y * 15.0; // multiply with sth for intensity
  // vig = pow(vig, 0.15); // change pow for modifying the extend of the  vignettea
  // out_color.rgb *= vig;

  vec3 tex_shells = texture(tex_shine_shells, v_uv).rgb;
  if(tex_shells.r > 0.0)
    out_color.rgb = lin_to_srgb(tex_shells);

  // ACES tonemap
  // out_color.rgb = Tonemap_ACES(out_color.rgb);

  out_color.a = 1.0f;
}