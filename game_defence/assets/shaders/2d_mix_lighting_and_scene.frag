#version 460

out vec4 out_color;
out vec4 out_bright_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos;
in vec2 v_sprite_wh;
in vec2 v_sprite_max;
in float v_tex_unit;
in vec2 v_vertex;  

uniform sampler2D scene;
uniform sampler2D lighting;
uniform float brightness_threshold = 0.8;
uniform vec2 light_pos; // worldspace
uniform vec2 camera_pos;
uniform vec2 viewport_wh;

float
SRGBFloatToLinearFloat(const float f)
{
  if (f <= 0.04045f)
    return f / 12.92f;
  return pow((f + 0.055f) / 1.055f, 2.4f);
};

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

float sdCircle( vec2 p, float radius ) 
{
    return length(p) - radius;
}

// shadow and light

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
  const vec3 scene = texture(scene, v_uv).rgb;
  const vec3 lighting = texture(lighting, v_uv).rgb;

  // convert uv to -1 and 1
  vec2 uv = (2.0 * v_uv - 1.0);
  uv.x *= -1;
  uv.y *= -1;

  float aspect = viewport_wh.x / viewport_wh.y;
  uv.x *= aspect;

  // convert worldspace to between -1 and 1.
  vec2 half_wh = viewport_wh / 2.0;
  float screen_min_x = 0; // e.g. -960
  // float screen_max_x = camera_pos.x + half_wh.x; // e.g. 960
  float screen_min_y = 0; // e.g. -540
  // float screen_max_y = camera_pos.y + half_wh.y; // e.g. 540

  float ss_x = (((light_pos.x - screen_min_x)/viewport_wh.x) * 2.0) - 1.0;
  float ss_y = (((light_pos.y - screen_min_y)/viewport_wh.y) * 2.0) - 1.0;
  ss_x *= aspect;

  vec2 p = vec2(uv.x + ss_x, uv.y + ss_y);

  // draw sdf at light_pos
  float d = sdCircle(p, 0.1);
	vec4 light1Col = vec4(0.75, 1.0, 0.5, 1.0);
  setLuminance(light1Col, 0.4);

  // linear to srgb
  vec3 corrected = lin_to_srgb(scene);

  vec3 col = (d>0.0) ? vec3(0.0,0.0,0.0) : vec3(0.25,0.25,0.25); // colour
  out_color.rgb = corrected;
  out_color.rgb += col; // additive
  out_color.a = 1.0f;

  // work out "bright" areas for bloom effect
  float brightness = luminance(out_color.rgb);
  vec4 bright_colour = vec4(0.0, 0.0, 0.0, 1.0);
  if(brightness > brightness_threshold)
    bright_colour = vec4(out_color.rgb, 1.0);
  out_bright_color = bright_colour;

  // after lighting, clamp buffer
	out_color.rgb = clamp(out_color.rgb, 0.0, 1.0);
}