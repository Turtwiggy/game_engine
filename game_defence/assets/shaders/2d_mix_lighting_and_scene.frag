#version 460

out vec4 out_color;

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

  // distance 200
  // const float light_linear = 0.022f;
  // const float light_quadratic = 0.0019f;
  // distance 325
  // const float light_linear = 0.014f;
  // const float light_quadratic = 0.0007f;
  // distance 600
  // const float light_linear = 0.007f;
  // const float light_quadratic = 0.0002f;
  // distance 3250
  // const float light_linear = 0.0014f;
  // const float light_quadratic =  0.000007f;

  const float d = length(light_pos - v_vertex);

  const float light_constant = 1.0f;
  const float linear =  0.0001f;
  const float quadratic = 0.00001f;
  const float attenuation = 1.0 / (light_constant + linear * d + quadratic * (d * d));

  // const float b_light_constant = 1.0f;
  // const float b_linear =  0.0001f;
  // const float b_quadratic = 0.00001f;
  // const float steep_attenuation = 1.0 / (b_light_constant + b_linear * d + b_quadratic * (d * d));

  const vec3 lighting_attenuated = (lighting.rgb * attenuation);
  const vec3 scene_corrected = lin_to_srgb(scene.rgb);
  const vec3 scene_attenuated = (scene_corrected* attenuation);

  // no lighting
  if(lighting.rgb == vec3(0.0f))
  {
    // no lighting, no scene
    if(scene_corrected.rgb == vec3(0.0f)){
      // out_color.rgb = vec3(1.0, 0.0, 0.0);
      // out_color.rgb = scene_corrected;

      // dark background, as in shadow
      const vec3 background_attenuated = lin_to_srgb(v_colour.rgb) * attenuation;
      out_color.rgb = background_attenuated * vec3(0.8);
    }
    // no lighting, yes scene
    else{
      // out_color.rgb = scene_corrected * lighting_attenuated;
      out_color.rgb = scene_attenuated;
    }
  }
  // yes lighting
  else
  {
    // yes lighting, no scene
    if(scene_corrected == vec3(0.0f))
    {
      const vec3 background_attenuated = lin_to_srgb(v_colour.rgb) * attenuation;
      out_color.rgb = background_attenuated;
    }
    // yes lighting, yes scene
    else
    {
      // show scene attenuated
      out_color.rgb = scene_attenuated;
    }
  }

  out_color.a = 1.0f;
}