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
						worley(p * 40.0 + 0.12 + -0.1*time)) *
							sqrt(
								sqrt(
									worley(p * 100.0 + 0.03*time)
								)
							)
						)
					)
			);
}
      
void main()
{
	vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  vec2 fragCoord = v_uv * viewport_wh;
  vec2 iResolution = viewport_wh;
	vec2 half_wh = viewport_wh / 2.0;

	// vec2 center = iResolution.xy * 0.5;
	// vec2 p = ((fragCoord - center) * zoom + center + vec2(0.5));
	// vec2 screen_min = camera_pos - half_wh; // e.g. -960, -540 for 1920x1080
	// vec2 screen_max = camera_pos + half_wh; // e.g. 960, 540 for 1920x1080

	vec2 centered = v_uv - 0.5;
	vec2 zoomed = centered * (zoom); // e.g. 2.0 for zoomout, 0.5 for zoomin
	vec2 final = zoomed + 0.5;
	vec2 uv = final;

	vec2 camera_uv_screen = vec2( 
		camera_pos.x / half_wh.x, 
		camera_pos.y / half_wh.y 
	);
  vec2 camera_uv = camera_uv_screen; 
	uv += camera_uv;
	uv += zoomed;

	//Calculate an intensity
	float t = fworley(uv * iResolution.xy/5000);	

	//Add some gradient
	// t*=exp(-length2(abs(0.2*uv - 1.0)));	
	t *= 0.4;

	//Make it blue!
	out_colour = vec4(t * vec3(0.1, 1.1*t, pow(t, 0.5-t)), 1.0);
}

