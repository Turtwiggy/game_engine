#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D tex;
uniform float time;

//  Perlin 2D Noise by Stefan Gustavson
vec2
fade(vec2 t)
{
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
vec4
permute(vec4 x)
{
  return mod(((x * 34.0) + 1.0) * x, 289.0);
}
vec4
taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}
vec3
fade(vec3 t)
{
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float
cnoise(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x, gy.x);
  vec2 g10 = vec2(gx.y, gy.y);
  vec2 g01 = vec2(gx.z, gy.z);
  vec2 g11 = vec2(gx.w, gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

float
posterize(float v, float k)
{
  return ceil(v * k) / k;
}

void
main()
{
  // Normalized pixel coordinates (from 0 to 1)
  vec2 uv = TexCoords;

  const float Y_SECTIONS = 7.0;

  float post = posterize(uv.y, Y_SECTIONS);

  vec2 offset_tex = uv + vec2(-time / 10.0, time / 10.0);
  // float noise = cnoise(offset_tex * 10.0) * 0.1;
  float noise = 0.0f;
  float invK = 1.0 / Y_SECTIONS;

  float bar = step(uv.y, post) - step(uv.y, post - noise / Y_SECTIONS);
  float c = (1.0 - bar) * post + bar * (post + invK); // vec3(0.2, 0.8, 0.8);

  vec3 color = vec3(c) * vec3(0.2, 0.8, 0.8);
  // color.xyz = vec3(noise) * vec3(1.0, 01.0, 0.0);

  FragColor = vec4(color, 1.0);
}
