// version prepended to file when loaded by engine.
//

// based on: 
// https://www.shadertoy.com/view/llS3RK

out vec4 out_colour;

in VS_OUT
{
  vec2 v_uv;
  vec4 v_colour;
  vec2 v_sprite_pos;  // x, y location of sprite
  vec2 v_sprite_wh;   // desired sprites e.g. 2, 2
  vec2 v_sprite_max;  // 22 sprites
  float v_tex_unit;
  vec2 v_vertex;
} fs_in;

layout(std140) uniform Data {
  mat4 projection_zoomed;
  mat4 view;
  vec2 camera_pos;
  float time;
  float zoom;
  float tilesize;
};

uniform vec2 viewport_wh;

//Calculate the squared length of a vector
float length2(vec2 p){
    return dot(p,p);
}

//Generate some noise to scatter points.
float noise(vec2 p){
	return fract(sin(fract(sin(p.x) * (43.13311)) + p.y) * 31.0011);
}

float worley(vec2 p) {
    //Set our distance to infinity
	float d = 1e30;
    //For the 9 surrounding grid points
	for (int xo = -1; xo <= 1; ++xo) {
		for (int yo = -1; yo <= 1; ++yo) {
            //Floor our vec2 and add an offset to create our point
			vec2 tp = floor(p) + vec2(xo, yo);
            //Calculate the minimum distance for this grid point
            //Mix in the noise value too!
			d = min(d, length2(p - tp - noise(tp)));
		}
	}
	return 3.0*exp(-4.0*abs(2.5*d - 1.0));
}

float fworley(vec2 p) {
	// Stack noise layers 
	return 
	sqrt(
		sqrt(
			sqrt(
				worley(p*5.0 + 0.05*time) *
					sqrt(
						worley(p * 50.0 + 0.12 + -0.1*time)) *
							sqrt(
								sqrt(
									// worley(p * 100.0 + 0.03*time)
									worley(p *- 10.0 + 0.03*time)
								)
							)
						)
					)
			);
}

float sdfCircle( in vec2 p, in float r ) 
{
    return length(p)-r;
}
      
// vec2 center = iResolution.xy * 0.5;
// vec2 p = ((fragCoord - center) * zoom + center + vec2(0.5));


void main()
{
	vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  vec2 fragCoord = v_uv * viewport_wh; // e.g. x 0>640, y 0>360
  vec2 iResolution = viewport_wh; 		 // e.g. 640, 360
	vec2 half_wh = viewport_wh / 2.0;

	// vec2 center = iResolution.xy * 0.5;
	// vec2 p = ((fragCoord - center) * zoom + center + vec2(0.5));
	vec2 screen_min = camera_pos - half_wh; // e.g. -960, -540 for 1920x1080
	vec2 screen_max = camera_pos + half_wh; // e.g. 960, 540 for 1920x1080

	float d = 0;
	{
		float aspect_x = viewport_wh.x / viewport_wh.y;

		// convert uv to -1 and 1
		vec2 tmp_uv = -(2.0 * v_uv - 1.0);
		tmp_uv.x *= aspect_x;
		tmp_uv *= zoom;

		vec2 pos = vec2(0, 0); // worldspace
		float tilesize = 64.0;
		float radius = 12.0;
		float size = tilesize * radius;

		// convert worldspace to between -1 and 1.
		vec2 ss = (((pos - screen_min)/viewport_wh) * 2.0) - 1.0;
		ss.x *= aspect_x;
		vec2 p = tmp_uv + ss;

		d = sdfCircle(p, size / (aspect_x * 100.0f));
		
		// vec3 col = (d>0.0) ? vec3(0.9,0.6,0.3) : vec3(0.65,0.85,1.0);
		// out_colour.rgb = col;
	}


	vec2 uv = v_uv; // uv between 0 and 1
	
	// Adjust for zoom, centering around (0.5, 0.5)
	uv -= 0.5;
	uv *= zoom; // e.g. zoom out > 1.0, zoom in with < 1.0
	uv += 0.5;

	vec2 camera_uv_screenspace = camera_pos / viewport_wh; // [0, 1]
	// vec2 camera_uv_clipspace = camera_uv_screenspace * 2.0 - 1.0; // [-1, 1]
	uv += camera_uv_screenspace;

	float t = fworley(uv * iResolution.xy / 1500.0);
	// t *= exp(-length2(abs(0.7*uv - 1.0))); // add gradient

	t *= d < 0.0 ? abs(d) * 0.85 : 0.0;
	t *= 0.2;
	// t = min(0.6, t);

	vec3 col = t * vec3(0.1, 1.1*t, pow(t, 0.5-t));
	out_colour.rgb = col;

	out_colour.a = 1.0f;
}

