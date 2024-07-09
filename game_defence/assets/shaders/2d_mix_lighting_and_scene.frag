#version 330 core

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos;
in vec2 v_sprite_wh;
in vec2 v_sprite_max;
in float v_tex_unit;
// in vec2 v_vertex;  

uniform sampler2D scene_0; // linear main
uniform sampler2D scene_1; // stars
uniform sampler2D lighting;
uniform float brightness_threshold = 0.8;
uniform vec2 light_pos; // worldspace
uniform vec2 mouse_pos;
uniform vec2 camera_pos;
uniform vec2 viewport_wh;
uniform float time;
uniform bool put_starshader_behind;

struct Line{
  vec2 start;
  vec2 end;
};
#define NR_LINES 100
uniform Line lines[NR_LINES];

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

float boxDist(vec2 p, vec2 size, float radius)
{
	size -= vec2(radius);
	vec2 d = abs(p) - size;
  	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - radius;
}

float lineDist(vec2 p, vec2 start, vec2 end, float width)
{
	vec2 dir = start - end;
	float lngth = length(dir);
	dir /= lngth;
	vec2 proj = max(0.0, min(lngth, dot((start - p), dir))) * dir;
	return length( (start - p) - proj ) - (width / 2.0);
}

float merge(float d1, float d2)
{
	return min(d1, d2);
}

// scene

float sceneDist(vec2 p)
{
  const float radius = 5;
	vec2 half_wh = viewport_wh / 2.0;
	vec2 screen_min = camera_pos - half_wh; // e.g. -960

	float c1 = circleDist(		translate(p, mouse_pos), 40.0);
	float c2 = circleDist(		translate(p, vec2(200, 250)), 40.0);

	float m = merge(c1, c2);

  // draw all the walls
  for(int i = 0; i < NR_LINES; i++){

    // values in worldspace
    Line l = lines[i]; 

    // should really check if something is active or not...
		// if(l.start == vec2(0.0, 0.0))
		// 	continue;
	
		vec2 a_corr = (l.start - screen_min);
		vec2 b_corr = (l.end  - screen_min);

    // line approach...
    float d = lineDist( p, a_corr, b_corr, radius);
    m = merge(m, d);
  }

  // draw all the shadows as circles
  // todo...

  return m;
}

float sceneSmooth(vec2 p, float r){
	float accum = sceneDist(p);
	accum += sceneDist(p + vec2(0.0, r));
	accum += sceneDist(p + vec2(0.0, -r));
	accum += sceneDist(p + vec2(r, 0.0));
	accum += sceneDist(p + vec2(-r, 0.0));
	return accum / 5.0;
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

float outerBorderMask(float dist, float width)
{
	//dist += 1.0;
	float alpha1 = clamp(dist, 0.0, 1.0);
	float alpha2 = clamp(dist - width, 0.0, 1.0);
	return alpha1 - alpha2;
}

// shadow and light

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

float luminance(vec3 col)
{
  return 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
}

void setLuminance(inout vec4 col, float lum)
{
	lum /= luminance(col.rgb);
	col *= lum;
}

float AO(vec2 p, float dist, float radius, float intensity)
{
	float a = clamp(dist / radius, 0.0, 1.0) - 1.0;
	return 1.0 - (pow(abs(a), 5.0) + 1.0) * intensity + (1.0 - intensity);
	return smoothstep(0.0, 1.0, dist / radius);
}

void main()
{
	out_color.a = 1.0f;
	out_bright_color.a = 1.0f;

	// disable bloom
  out_bright_color = vec4(1.0, 0.0, 0.0, 1.0);

  // linear to srgb
  vec4 scene = texture(scene_0, v_uv);
  vec3 stars = texture(scene_1, v_uv).rgb;
  vec3 lighting = texture(lighting, v_uv).rgb;
  vec3 corrected = lin_to_srgb(scene.rgb);

	if(put_starshader_behind)
		out_color.rgb = stars + corrected;
	else
		out_color.rgb = corrected;

	return;
  
	// fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis
  // iResolution : is a vec2 with an X value of 640 and a Y value of 360
  vec2 fragCoord = v_uv * viewport_wh;
  vec2 iResolution = viewport_wh;
  vec2 p = fragCoord.xy + vec2(0.5);
	vec2 c = iResolution.xy / 2.0;

  vec4 lightColBlue = vec4(0.5, 0.75, 1.0, 1.0);
	setLuminance(lightColBlue, 0.4);

  vec4 lightColOrange =  vec4(1.0, 0.75, 0.5, 1.0);
	setLuminance(lightColOrange, 0.5);

  vec4 lightColGreen = vec4(0.75, 1.0, 0.5, 1.0);
	setLuminance(lightColGreen, 0.6);

  // light
	vec4 light1Col = lightColGreen;
	vec4 light2Col = lightColOrange;
	vec4 light3Col = lightColBlue;

	vec2 half_wh = viewport_wh / 2.0;
	vec2 screen_min = camera_pos - half_wh; // e.g. -960

  vec2 light1Pos = light_pos.xy;
	vec2 light2Pos = vec2(iResolution.x * (sin(time + 3.1415) + 1.2) / 2.4, 500.0) - screen_min;
	vec2 light3Pos = vec2(iResolution.x * (sin(time) + 1.2) / 2.4, 100.0) - screen_min;
  // vec2 light4Pos = vec2(100, 100) - screen_min;
  // vec2 light5Pos = vec2(200, 200) - screen_min;
  // vec2 light6Pos = vec2(300, 300) - screen_min;

  float dist = sceneDist(p);

  // ambient + vignette
	vec4 col = vec4(0.5, 0.5, 0.5, 1.0) * (1.0 - length(c - p)/iResolution.x);

	// todo: generate ambient occlusion only when map is generated
  // ambient occlusion
	// col *= AO(p, sceneSmooth(p, 10.0), 40.0, 0.4);
	// col *= 1.0-AO(p, sceneDist(p), 40.0, 1.0);

  // light
	// col += drawLight(p, light1Pos, light1Col, dist, 150.0, 6.0);
  // col += drawLight(p, light2Pos, light2Col, dist, 200.0, 8.0);
	// col += drawLight(p, light3Pos, light3Col, dist, 300.0, 12.0);
	// col += drawLight(p, light4Pos, light1Col, dist, 400.0, 12.0);
	// col += drawLight(p, light5Pos, light1Col, dist, 500.0, 12.0);
	// col += drawLight(p, light6Pos, light1Col, dist, 600.0, 12.0);
  // shape fill
	col = mix(col, vec4(1.0, 0.4, 0.0, 1.0), fillMask(dist));
  // shape outline
	col = mix(col, vec4(0.1, 0.1, 0.1, 1.0), innerBorderMask(dist, 1.5));

  // out_color.rgb = col.rgb + corrected.rgb; // HMM
  // out_color.rgb = col.rgb;

  // work out "bright" areas for bloom effect
  float brightness = luminance(out_color.rgb);
  vec4 bright_colour = vec4(0.0, 0.0, 0.0, 1.0);
  if(brightness > brightness_threshold)
    bright_colour = vec4(out_color.rgb, 1.0);
  out_bright_color = bright_colour;

  // after lighting, clamp buffer
	out_color.rgb = clamp(out_color.rgb, 0.0, 1.0);
  out_color.a = 1.0f;
}