// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_size; // e.g. 16, 16
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  vec2 v_sprite_global_pos;
  float v_tex_unit;
} fs_in;

layout(std140) uniform Data {
  mat4 projection_zoomed;
	mat4 view;
	vec4[32] light_positions;
  vec2 camera_pos;
  vec2 screenshake;
  float time;
  float zoom;
  float tilesize;
};

uniform vec2 viewport_wh;

// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/
float field(in vec3 p) {
	float strength = 7. + .03 * log(1.0 + fract(sin(time) * 4373.11));
	float accum = 0.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 32; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		// accum += w * exp(-strength * pow(abs(mag - prev), 2.3));
		accum += w * exp(-strength * pow(abs(mag - prev), 2.0));
		tw += w;
		prev = mag;
	}
  return max(0., 5. * accum / tw - .7);
	// return max(0.0, 5. * accum / tw - .8);
}

// https://www.shadertoy.com/view/lslGWr

void main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour  = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  vec2 cam_uv = camera_pos + (v_uv * viewport_wh);
	cam_uv /= viewport_wh;

  vec2 uv = cam_uv;
  // float iTime = time * 0.0; // game speed
  float iTime = time * 1.0; // menu speed
  vec2 iResolution = viewport_wh; 		 // e.g. 640, 360

	vec2 uvs = cam_uv * iResolution.xy / max(iResolution.x, iResolution.y);
	vec3 p = vec3(uvs / 4., 0) + vec3(1., -1.3, 0.);
	p += .2 * vec3(sin(iTime / 16.), sin(iTime / 12.),  sin(iTime / 128.));
	float t = field(p);
   
	out_colour = mix(.4, 1., 1.0) * vec4(1.8 * t * t * t, 1.4 * t * t, t, 1.0);
  // out_colour.r *= 2.0f;
}