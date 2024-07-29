// version prepended to file when loaded by engine.
// #version 130

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D tex;

uniform bool horizontal;

// gaussian blur
void
main()
{
  const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

  ivec2 tex_size = textureSize(tex, 0);
  vec2 tex_offset = vec2(1.0) / vec2(tex_size.x, tex_size.y); // gets size of single texel
  vec3 result = texture(tex, v_uv).rgb * weight[0]; // current fragment's contribution
  if(horizontal)
  {
    for(int i = 1; i < 5; ++i)
    {
      result += texture(tex, v_uv + vec2(tex_offset.x * float(i), 0.0)).rgb * vec3(weight[i]);
      result += texture(tex, v_uv - vec2(tex_offset.x * float(i), 0.0)).rgb * vec3(weight[i]);
    }
  }
  else
  {
    for(int i = 1; i < 5; ++i)
    {
      result += texture(tex, v_uv + vec2(0.0, tex_offset.y * float(i))).rgb * vec3(weight[i]);
      result += texture(tex, v_uv - vec2(0.0, tex_offset.y * float(i))).rgb * vec3(weight[i]);
    }
  }

  out_color = vec4(result, 1.0);
}