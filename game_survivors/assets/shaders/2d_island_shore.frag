// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_vertex;
} fs_in;

layout(std140) uniform Data {
  mat4 projection_zoomed;
	mat4 view;
  vec2 camera_pos;
	vec4[32] light_positions;
  float time;
  float zoom;
  float tilesize;
};

float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}

float cnoise(vec2 P){
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
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
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

vec4 rgb(float r, float g, float b) {
	return vec4(r / 255.0, g / 255.0, b / 255.0, 1.0);
}

#define PI 3.14159265359

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;

  vec2 uv = (vec2(1.0) - v_uv);
  float iTime = time;

	vec4 snd = rgb(231., 213., 198.);
	vec4 wtr = rgb(0., 0., 0.);
	vec4 wtr2 = rgb(0., 0., 0.);

  float sltm = iTime * 0.2;

	float uvy = uv.y - (sin(iTime) * 0.5 + 0.5) * 0.1 - 0.79;
	float wuvy = uv.y - (sin(.75) * 0.5 + 0.5) * 0.1 - 0.78;

	float shore = sin(uv.x * PI * 4. + sltm);
	shore += sin(uv.x * PI * 3.);
	shore = shore * 0.5 + 0.5;
	shore *= 0.05;
	float smshore = smoothstep( uvy * 5., uvy * 5. + 2.5, shore);
	float wshore = smoothstep( wuvy * 5., wuvy * 5., shore);

	shore = smoothstep( uvy * 5., uvy * 5. + 2., shore);

	float shmsk = step(0.01, shore);
	float shmsko = step(0.012, shore);
    
  float suvx = uv.x + (uv.y * 5.);
	float sand = step(fract(uv.y * 10.) * 2. - 0.5, (sin(suvx * PI * 1.5)
															+ sin(suvx * PI * 2.)) * 0.5 + 0.5);
	sand -= step(fract(uv.y * 10.) * 2., sin(suvx * PI * 2.) * 0.5 + 0.5);

	wtr = mix(wtr2, wtr, smoothstep (0.0, 0.5, uv.y));
	snd *= clamp(sand, 0.95, 1.);
	vec4 res = mix(snd, wtr, smshore);
    
  // cnoise(vec2(uv.x, uvy * 4. + sltm * 0.5) * 10.0) * 0.5 + 0.5
  // float foam = cnoise(vec2(uv.x * 1, uvy * 4. + sltm * 0.2) * 10.0) * 0.5 + 0.1;
  float foam = 1.0f;
  foam = distance(v_uv, vec2(0.5));

	float ofoam = step(shore + 0.05, foam) * shmsk;
	foam = step(shore, foam) * shmsk;

  //vec2(uv.x, uvy * 4. + soff + sltm * 0.5)
	float soff = mix(0.01, 0.2, smoothstep (0.7, 0., uv.y));
	float foams = cnoise(vec2(uv.x * 3.0, uvy * 4. + soff + sltm * 0.5) * 10) * 0.5 + 0.5;
	foams = 1. - step(shore, foams) * shmsk * 0.2;
	res *= foams;
	res *= (1. - wshore * (1. - shmsk) * (sin(iTime - PI / 2.) * 0.5 + 0.5) * 0.2);
	res = mix(res, vec4(1.), foam);

  out_colour = res;

  if(length(out_colour.rg) < 0.8)
    out_colour.a = 0.0f;
}