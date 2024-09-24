// version prepended to file when loaded by engine.
//

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos;
in vec2 v_sprite_wh;
in vec2 v_sprite_max;
in float v_tex_unit;

uniform sampler2D scene_0;         // linear main
uniform sampler2D scene_1; 		 	   // stars
uniform sampler2D u_distance_data; // distance data
uniform sampler2D tex_unit_debris;
uniform sampler2D tex_unit_floor_mask;

uniform float brightness_threshold;
uniform vec2 camera_pos;
uniform vec2 viewport_wh;
uniform bool put_starshader_behind;
uniform bool add_grid;
uniform vec2 uv_offset;
uniform bool inside_spaceship;
uniform float tilesize;
uniform float zoom;

#define NR_MAX_CIRCLES 100
uniform samplerBuffer circleBuffer;  // a texture buffer

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

// https://www.shadertoy.com/view/lt3BW2
float opSmoothUnion( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
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
	float m = V2_F16(texture(u_distance_data, uv).rg) * viewport_wh.y * m_sign;

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
		// lf = min(lf, sd / dt);
		lf = min(lf, (sd / dt));
		
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
	return (shad * fall) * color;
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

	// fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis
  // iResolution : is a vec2 with an X value of 640 and a Y value of 360
  vec2 fragCoord = (v_uv * viewport_wh);
  vec2 iResolution = viewport_wh;
	vec2 p = (fragCoord + vec2(0.5));
	vec2 c = iResolution.xy / 2.0;
	
	vec2 half_wh = viewport_wh / 2.0;
	vec2 screen_min = camera_pos - half_wh; // e.g. -960

  vec4 scene_lin = texture(scene_0, v_uv);
  vec3 stars_srgb = texture(scene_1, v_uv).rgb;

	if(put_starshader_behind){
		vec3 scene_col = lin_to_srgb(scene_lin.rgb);
		out_color.rgb = stars_srgb.rgb + scene_col;
		return;
	}

	float dist = sceneDist(p);

	// gradient
	// vec4 col = vec4(0.3, 0.3, 0.3, 1.0) * (1.0 - length(c - p)/iResolution.x);

	// inside spaceship

	vec4 col = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	if(inside_spaceship)
	{
		col = vec4(0.3f, 0.3f, 0.3f, 1.0f);
		col *= AO(dist, 40.0f, 1.0f);
	}
	else
	{
		// outside spaceship
		col = vec4(0.3f, 0.3f, 0.3f, 1.0f);
		col *= 1.0f - AO(dist, 1.0f, 0.8f);
	}

	// lights
	//
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		Light l = lights[i];

		if(!l.enabled){
			continue;
		}

		// inside spaceship
		// most lights, 0.6
		// player light: 1.0
		// player light outside spaceship: 1.25
 		setLuminance(l.colour, l.luminance);

		// vec4 pcol = vec4(1.0f); 

		col += drawLight(p, l.position, l.colour, dist, 450.0, 12.0);
	}
	col = clamp(col, 0.0, 1.0);

	// sdf circles for oxygen
	//
	vec3 circle_col = vec3(1.0f);
	{
		// convert uv to -1 and 1
		vec2 uv = (2.0 * v_uv - 1.0);
		uv.x *= -1.0;
		uv.y *= -1.0;
		float aspect = viewport_wh.x / viewport_wh.y;
		uv.x *= aspect;

  	float d = 1e10;

		for (int i = 0; i < NR_MAX_CIRCLES; ++i) {
			vec3 circleData = texelFetch(circleBuffer, i).xyz;
			vec2 pos = circleData.xy;   // Circle center position
			float radius = circleData.z;       // Circle radius

			if(pos == vec2(0.0))
				break; // assume no more circles

			// convert worldspace to between -1 and 1.
			float ss_x = (((pos.x - screen_min.x)/viewport_wh.x) * 2.0) - 1.0;
			float ss_y = (((pos.y - screen_min.y)/viewport_wh.y) * 2.0) - 1.0;
			ss_x *= aspect;

			vec2 p = vec2(uv.x + ss_x, uv.y + ss_y);

			// radius 
			float circle_size = tilesize * radius;
			float rad = (circle_size / viewport_wh.y);	
			float d0 = sdCircle(p, rad);

			// If not the first circle, smooth it in
			float dt = opSmoothUnion(d, d0, 0.1);
			d = min(d, dt); 
		}

		// colouring
		vec3 ccol = vec3(0.0f, 0.4f, 0.4f);

		float thickness = 0.0025;
		ccol *= mix( vec3(0.0), vec3(1.0), 1.0-smoothstep(0.0,thickness,abs(d)) ); // border
		circle_col.rgb = ccol;
	}

  // linear to srgb
	vec3 final_lin = scene_lin.rgb;

	// debris backdrop
	vec3 scene_debris_lin = texture(tex_unit_debris, v_uv).rgb;
	float floor_mask = texture(tex_unit_floor_mask, v_uv).r;
	// bool black_debris = scene_debris_lin == vec3(0.0f);
	if(floor_mask >= 0.95 ){
		scene_debris_lin = vec3(0.0f);
	}
	final_lin += scene_debris_lin;

	// sdf grid	
	//
	if(add_grid) {
		vec3 grid_col = vec3(0.0f);
		{
			float gridsize = tilesize / zoom; // pixels
			vec2 camera_uv_screen = vec2( camera_pos.x / half_wh.x, camera_pos.y / half_wh.y); // camera position is in worldspace.
			vec2 camera_uv = camera_uv_screen / zoom; 
			float aspect_y = viewport_wh.y / viewport_wh.x;
			
			vec2 grid_uv = (2.0 * v_uv - 1.0);
			grid_uv += camera_uv;
			grid_uv.y *= aspect_y;

			vec2 p_grid = (viewport_wh.x / gridsize / 2.0) * grid_uv;

			// if the gridsize gets too small and the gridwidth isnt large enough, 
			// the grid appears to dissapear. the value 0.05 seems to work until gridsize<10
			float grid_width = 0.02; 
			float margin = 0.5;
			if(abs(sdGrid(p_grid, margin)) >= grid_width)
				grid_col = vec3(0.0f);// background
			else
				grid_col = vec3(0.25); // line
		}
		vec3 grid_lin = srgb_to_lin(vec3(grid_col.r * 255.0f, grid_col.g * 255.0f, grid_col.b * 255.0f));
		final_lin += grid_lin;
	}

	// lighting
	vec3 lighting_lin = srgb_to_lin(vec3(col.r * 255.0f, col.g * 255.0f, col.b * 255.0f));
	final_lin *= lighting_lin;

	vec3 srgb_final = lin_to_srgb(final_lin);
	// vec3 srgb_final = lin_to_srgb(scene_lin.rgb);

	// out_color.rgb = circle_col + srgb_final.rgb;
	out_color.rgb = srgb_final.rgb;

	// vignette
	vec2 vig_uv = fragCoord.xy / iResolution.xy;
	vig_uv *=  1.0 - vig_uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
	float vig = vig_uv.x*vig_uv.y * 15.0; // multiply with sth for intensity
	vig = pow(vig, 0.15); // change pow for modifying the extend of the  vignettea
	out_color.rgb *= vig;

	out_color.a = 1.0f;
}