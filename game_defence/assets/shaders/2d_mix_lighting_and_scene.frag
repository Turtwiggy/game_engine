#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform vec2 screen_wh;

uniform sampler2D lighting;
uniform sampler2D scene;

// vec3 lin_to_srgb(vec3 color)
// {
//   vec3 x = color.rgb * 12.92;
//   vec3 y = 1.055 * pow(clamp(color.rgb, 0.0, 1.0), vec3(0.4166667)) - 0.055;
//   vec3 clr = color.rgb;
//   clr.r = (color.r < 0.0031308) ? x.r : y.r;
//   clr.g = (color.g < 0.0031308) ? x.g : y.g;
//   clr.b = (color.b < 0.0031308) ? x.b : y.b;
//   return clr.rgb;
// }

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
  vec3 mixed = mix(texture(lighting, v_uv).rgb, texture(scene, v_uv).rgb, 0.75f);

  out_color = vec4(lin_to_srgb(mixed), 1.0f);
}