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
uniform vec2 light_pos; // worldspace
uniform float brightness_threshold;
uniform float exposure = 1.0f;

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

void main()
{
  const vec3 scene = texture(scene, v_uv).rgb;
  const vec3 lighting = texture(lighting, v_uv).rgb;

  // linear to srgb
  out_color.rgb = lin_to_srgb(scene);
  out_color.a = 1.0f;

  // work out "bright" areas for bloom effect
  float brightness = dot(out_color.rgb, vec3(0.2126, 0.7152, 0.0722));
  vec4 bright_colour = vec4(0.0, 0.0, 0.0, 1.0);
  if(brightness > brightness_threshold)
    bright_colour = vec4(out_color.rgb, 1.0);
  out_bright_color = bright_colour;
}