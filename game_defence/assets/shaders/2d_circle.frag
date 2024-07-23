#version 300 es
precision highp float;

layout(location = 0) out vec4 o_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform vec2 viewport_wh;
uniform vec2 camera_pos;

struct Points
{
  vec2 pos;
};
#define NR_MAX_CIRCLES 20
uniform Points points[NR_MAX_CIRCLES];

// todo: move to v
// float thickness = 0.05;
// float fade = 0.1;

// https://www.shadertoy.com/view/3ltSW2
float sdCircle( vec2 p, float radius ) 
{
    return length(p) - radius;
}

// https://www.shadertoy.com/view/lt3BW2
float opSmoothUnion( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

void
main()
{
  vec2 half_wh = viewport_wh / 2.0;
  float screen_min_x = camera_pos.x - half_wh.x; // e.g. -960
  float screen_max_x = camera_pos.x + half_wh.x; // e.g. 960
  float screen_min_y = camera_pos.y - half_wh.y; // e.g. -540
  float screen_max_y = camera_pos.y + half_wh.y; // e.g. 540
  // float range_x = screen_max_x - screen_min_x;
  // float range_y = screen_max_y - screen_min_y;

  // convert uv to -1 and 1
  vec2 uv = (2.0 * v_uv - 1.0);
  uv.x *= -1.0;
  uv.y *= -1.0;

  float aspect = viewport_wh.x / viewport_wh.y;
  uv.x *= aspect;

  float d = 1e10;

  for(int i = 0; i < NR_MAX_CIRCLES; i++)
  {
    // values in worldspace
    vec2 pos = points[i].pos;         
    
    // should really check if something is active or not...
    if(pos == vec2(0.0)) 
      continue;

    // convert worldspace to between -1 and 1.
    float ss_x = (((pos.x - screen_min_x)/viewport_wh.x) * 2.0) - 1.0;
    float ss_y = (((pos.y - screen_min_y)/viewport_wh.y) * 2.0) - 1.0;
    ss_x *= aspect;

    vec2 p = vec2(uv.x + ss_x, uv.y + ss_y);
    float d0 = sdCircle(p, 0.2);

    // If not the first circle, smooth it in
    float dt = opSmoothUnion(d, d0, 0.1);
    d = min(d, dt);
  }

  // colouring
  // vec3 col = (d>0.0) ? vec3(0.9,0.6,0.3) : vec3(0.65,0.85,1.0); // colour
	// col *= 0.8 + 0.2*cos(150.0*d);                                // waves
  // col *= 1.0 - exp(-6.0*abs(d));                                // shadow

  // float thickness = 0.0015;
  float thickness = 0.005;
	vec3 col = mix( vec3(0.0), vec3(1.0), 1.0-smoothstep(0.0,thickness,abs(d)) ); // border

  o_colour = vec4(col, 1.0);

  if(col == vec3(0.0))
    o_colour.a = 0.0;
  else
    o_colour = vec4(0.5f);
}