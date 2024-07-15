#version 330 core

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos;
in vec2 v_sprite_wh;
in vec2 v_sprite_max;
in float v_tex_unit;

uniform sampler2D scene_0;     // linear main
uniform sampler2D scene_1; 		 // stars
uniform sampler2D u_distance_data; // distance data

uniform float brightness_threshold = 0.8;
uniform vec2 light_pos; // worldspace
uniform vec2 mouse_pos;
uniform vec2 camera_pos;
uniform vec2 viewport_wh;
uniform float iTime;
uniform bool put_starshader_behind;

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

// masks for drawing

float fillMask(float dist)
{
	return clamp(-dist, 0.0, 1.0);
}

float innerBorderMask(float dist, float width)
{
	//dist += 1.0;
	float alpha1 = clamp(dist + width, 0.0, 1.0);
	float alpha2 = clamp(dist, 0.0, 1.0);
	return alpha1 - alpha2;
}


// the scene

float V2_F16(vec2 v) { return v.x + (v.y / 255.0); }
float sceneDist(vec2 p)
{
	vec2 uv = p / viewport_wh;
	
	float m_sign = texture(u_distance_data, uv).b;
	float m = V2_F16(texture(u_distance_data, uv).rg) * 720 * m_sign;

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

// lighting and shadows

float shadow(vec2 p, vec2 pos, float radius)
{
	vec2 dir = normalize(pos - p);
	float dl = length(p - pos);
	
	// fraction of light visible, starts at one radius (second half added in the end);
	float lf = radius * dl;
	
	// distance traveled
	float dt = 0.01;

	for (int i = 0; i < 64; ++i)
	{				
		// distance to scene at current position
		float sd = sceneDist(p + dir * dt);

        // early out when this ray is guaranteed to be full shadow
        if (sd < -radius) 
            return 0.0;
        
		// width of cone-overlap at light
		// 0 in center, so 50% overlap: add one radius outside of loop to get total coverage
		// should be '(sd / dt) * dl', but '*dl' outside of loop
		lf = min(lf, sd / dt);
		
		// move ahead
		dt += max(1.0, abs(sd));
		if (dt > dl) break;
	}

	// multiply by dl to get the real projected overlap (moved out of loop)
	// add one radius, before between -radius and + radius
	// normalize to 1 ( / 2*radius)
	lf = clamp((lf*dl + radius) / (2.0 * radius), 0.0, 1.0);
	lf = smoothstep(0.0, 1.0, lf);
	return lf;
}

vec4 drawLight(vec2 p, vec2 pos, vec4 color, float dist, float range, float radius)
{
	// distance to light
	float ld = length(p - pos);
	
	// out of range
	if (ld > range) return vec4(0.0);
	
	// shadow and falloff
	float shad = shadow(p, pos, radius);
	float fall = (range - ld)/range;
	fall *= fall;
	float source = fillMask(circleDist(p - pos, radius));
	return (shad * fall + source) * color;
}

float luminance(vec4 col)
{
	return 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
}

void setLuminance(inout vec4 col, float lum)
{
	lum /= luminance(col);
	col *= lum;
}

// dist will be a value between 0 and 1

float AO(float dist, float radius, float intensity)
{
	float a = clamp(dist / radius, 0.0, 1.0) - 1.0;
	return 1.0 - (pow(abs(a), 5.0) + 1.0) * intensity + (1.0 - intensity);
	return smoothstep(0.0, 1.0, dist / radius);
}


void main()
{
	out_color.a = 1.0f;

	// disable bloom
  out_bright_color = vec4(0.0, 0.0, 0.0, 1.0);

  // linear to srgb
  vec4 scene_lin = texture(scene_0, v_uv);
  vec3 stars = texture(scene_1, v_uv).rgb;

	if(put_starshader_behind){
		out_color.rgb = stars.rgb + lin_to_srgb(scene_lin.rgb);
		return;
	}

	// fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis
  // iResolution : is a vec2 with an X value of 640 and a Y value of 360
  vec2 fragCoord = v_uv * viewport_wh;
  vec2 iResolution = viewport_wh;
  vec2 p = fragCoord.xy + vec2(0.5);
	vec2 c = iResolution.xy / 2.0;

	float dist = sceneDist(p);
	// float dist = sceneSmooth(p, 5.0);

	// the dist at the edges of the shape is 0
	// the dist at the center of the shape is negative, 
	// increasing the furhter from edges
	// the dist away from the shape is
	/*
	if(dist > 0){
		out_color = vec4(dist, dist, dist, 1.0f);
		return;
	}
	if(dist == 0.0) {
		out_color = vec4(0.0, 0.0, 0.0f, 1.0f);
		return;
	}
	if(dist < 0){
		out_color = vec4(1.0, 0.0, 0.0, 1.0f);
		return;
	}
	*/

  // vec4 lightColGreen = vec4(0.6, 0.6, 1.0, 1.0);
	// setLuminance(lightColGreen, 1.0);

  // vec4 lightColOrange =  vec4(1.0, 0.75, 0.5, 1.0);
	// setLuminance(lightColOrange, 0.5);

	// vec4 lightColBlue = vec4(0.5, 0.75, 1.0, 1.0);
	// setLuminance(lightColBlue, 0.4);

	vec2 half_wh = viewport_wh / 2.0;
	vec2 screen_min = camera_pos - half_wh; // e.g. -960

	// gradient
	// vec4 col = vec4(0.0, 0.0, 0.0, 1.0) * (1.0 - length(c - p)/iResolution.x);

	// optiona a
	// vec4 col = vec4(0.5, 0.5, 0.5, 1.0);
	// col *= AO(sceneDist(p), 40.0, 1.0);

	// option b
	 vec4 col = vec4(0.0, 0.0, 0.0, 1.0);
	//  col *= AO(sceneSmooth(p, 10.0), 40.0, 1.0);	

	// light
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		Light l = lights[i];

		if(!l.enabled){
			continue;
		}

 		setLuminance(l.colour, 0.5);

		col += drawLight(p, l.position, l.colour, dist, 300.0, 1.0);
	}

	// shape fill
	// col = mix(col, vec4(1.0, 0.4, 0.0, 1.0), fillMask(dist));
	// shape outline
	// col = mix(col, vec4(0.1, 0.1, 0.1, 1.0), innerBorderMask(dist, 0.15));
	
	// vec3 lin_lighting = srgb_to_lin(col.rgb);
	// vec3 lin_all = lin_lighting + scene_lin.rgb;
	// vec3 srgb_final = lin_to_srgb(lin_all);

	vec3 srgb_final = col.rgb * lin_to_srgb(scene_lin.rgb);

	out_color.rgb = srgb_final.rgb;
	out_color.a = col.a;
	return;

  // work out "bright" areas for bloom effect
  float brightness = luminance(out_color);
  vec4 bright_colour = vec4(0.0, 0.0, 0.0, 1.0);
  if(brightness > brightness_threshold)
    bright_colour = vec4(out_color.rgb, 1.0);
  out_bright_color = bright_colour;

  // after lighting, clamp buffer
	out_color.rgb = clamp(out_color.rgb, 0.0, 1.0);
  out_color.a = 1.0f;
}