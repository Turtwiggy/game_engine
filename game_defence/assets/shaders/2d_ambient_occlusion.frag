#version 330 core

layout(location = 0) out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform sampler2D u_distance_data;
uniform sampler2D u_emitters_and_occluders;
uniform vec2 viewport_wh;
uniform vec2 mouse_pos;

// sdf translation

vec2 translate(vec2 p, vec2 t)
{
	return p - t;
}

// distance field functions

float circleDist( vec2 p, float radius ) 
{
    return length(p) - radius;
}

float merge(float d1, float d2)
{
	return min(d1, d2);
}

// ----------------------------


float V2_F16(vec2 v) { return v.x + (v.y / 255.0); }

float sceneDist(vec2 p)
{
  // float d = length(texture(u_emitters_and_occluders, v_uv).rgb);
	float d = V2_F16(texture2D(u_distance_data, v_uv).rg);

	float c = circleDist(		translate(p, mouse_pos), 40.0);

	float m = merge(d, c);

  return m;
}

float sceneSmooth(vec2 p, float r)
{
	float accum = sceneDist(p);
	accum += sceneDist(p + vec2(0.0, r));
	accum += sceneDist(p + vec2(0.0, -r));
	accum += sceneDist(p + vec2(r, 0.0));
	accum += sceneDist(p + vec2(-r, 0.0));
	return accum / 5.0;
}

float AO(vec2 p, float dist, float radius, float intensity)
{
	float a = clamp(dist / radius, 0.0, 1.0) - 1.0;
	return (pow(abs(a), 5.0) + 1.0) * intensity + (1.0 - intensity);
	return smoothstep(0.0, 1.0, dist / radius);
}

void main()
{
	// fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis
  // iResolution : is a vec2 with an X value of 640 and a Y value of 360
  vec2 fragCoord = v_uv * viewport_wh;
  vec2 iResolution = viewport_wh;
  vec2 p = fragCoord.xy + vec2(0.5);
	vec2 c = iResolution.xy / 2.0;

	vec4 col = vec4(0.1, 0.1, 0.1, 1.0);
	col *= AO(p, sceneSmooth(p, 10.0), 40.0, 0.9);

	out_colour = col;
}