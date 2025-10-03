// version prepended to file when loaded by engine.
//

// based on: 
// https://www.shadertoy.com/view/llS3RK
// ibreakdownshaders.blogspot.com

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
	vec4[32] light_positions;
  float time;
  float zoom;
  float tilesize;
};

uniform vec2 viewport_wh;

// use the fluid sim as a mask for the worley noise shader.
uniform int tex_fluid_sim;

//Calculate the squared length of a vector
float length2(vec2 p){
    return dot(p,p);
}

//Generate some noise to scatter points.
float noise(vec2 p){
	return fract(sin(fract(sin(p.x) * (43.13311)) + p.y) * 31.0011);
}
vec2 hash(vec2 P)
{
 	return fract(cos(P*mat2(-64.2,71.3,81.4,-29.8))*8321.3); 
}
vec2 hash2( vec2 p )
{
	// texture based white noise
	// return textureLod( iChannel0, (p+0.5)/256.0, 0.0 ).xy;

	// procedural white noise	
	return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

float worley(vec2 p) {

	float d = 1.; 
	vec2 ip = floor(p);
	vec2 fp = fract(p);


	for(int xo = -1; xo<=1; xo++)
	for(int yo = -1; yo<=1; yo++)
	{
		// Set a point as a grid-like structure
		vec2 tp = ip + vec2(xo, yo) ;

		// float dist = distance(hash(tp)+vec2(xo, yo), fp);
		float dist = length2(p - tp - noise(tp));

		d = min(d,dist);
	}

	// Round out the edges.
	// original: 
	// return 3.0*exp(-4.0*abs(2.0*d - 1.0));
	// shadertoy:
	return 3.0*exp(-4.0*abs(2.5*d - 1.0));
}

float fworley(vec2 p) {
	// Stack noise layers 

	// Spreading out the light can be done by taking repeated roots.
	// This changes the peaking function to a much more smooth one.

	float a_scale = 5; // sparse
	float a_time = 0.025*time;
	float b_scale = 50; // dense
	float b_flat = 0.12;
	float b_time = -0.1*time;
	float c_scale = -10; // dense
	float c_time = -0.03*time;

	float a = worley(p*a_scale + 					a_time); 			
	float b = worley(p*b_scale + b_flat + b_time);
	float c = worley(p*c_scale + 				  c_time);
	float d = sqrt(sqrt(sqrt( a * sqrt(b) * sqrt(sqrt(c)) )));

	return d;
}

float sdfCircle( in vec2 p, in float r ) 
{
    return length(p)-r;
}

// Compact, self-contained version of IQ's 2D value noise function.
float n2D(vec2 p){
   
    // Setup.
    // Any random integers will work, but this particular
    // combination works well.
    const vec2 s = vec2(1, 113);
    // Unique cell ID and local coordinates.
    vec2 ip = floor(p); p -= ip;
    // Vertex IDs.
    vec4 h = vec4(0., s.x, s.y, s.x + s.y) + dot(ip, s);
   
    // Smoothing.
    p = p*p*(3. - 2.*p);
    //p *= p*p*(p*(p*6. - 15.) + 10.); // Smoother.
   
    // Random values for the square vertices.
    h = fract(sin(mod(h, 6.2831589))*43758.5453);
   
    // Interpolation.
    h.xy = mix(h.xy, h.zw, p.y);
    return mix(h.x, h.y, p.x); // Output: Range: [0, 1].
}
// FBM -- 4 accumulated noise layers of modulated amplitudes and frequencies.
float fbm(vec2 p){ return n2D(p)*.533 + n2D(p*2.)*.267 + n2D(p*4.)*.133 + n2D(p*8.)*.067; }
      
void main()
{
	vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  vec2 v_vertex = fs_in.v_vertex;
  int index = int(fs_in.v_tex_unit);

  vec2 fragCoord = v_uv * viewport_wh; // e.g. x 0>640, y 0>360
  vec2 iResolution = viewport_wh; 		 // e.g. 640, 360
	vec2 half_wh = viewport_wh * 0.5;

	// wobble the uvs.
	float iTime = time;
	vec2 offs = vec2(fbm(v_uv*16.), fbm(v_uv*16. + .35));
	vec2 offs2 = vec2(fbm(v_uv*1. + iTime/4.), fbm(v_uv*1. + .5 + iTime/4.));

	// const float oFct = 0.5;
	// const float oFct2 = .1;

	const float oFct = .025;
	const float oFct2 = .02;

	v_uv -= (offs - .5)*oFct;
	v_uv -= (offs2 - .5)*oFct2;

	// vec2 center = iResolution.xy * 0.5;
	// vec2 p = ((fragCoord - center) * zoom + center + vec2(0.5));
	vec2 screen_min = camera_pos - half_wh; // e.g. -960, -540 for 1920x1080
	vec2 screen_max = camera_pos + half_wh; // e.g. 960, 540 for 1920x1080

	float d = 0.0;
	float aspect_x = viewport_wh.x / viewport_wh.y;
	{
		// convert uv to -1 and 1
		// the -1 is to invert the sdf
		vec2 tmp_uv = -1 * (2.0 * v_uv - 1.0);
		tmp_uv.x *= aspect_x;
		tmp_uv *= zoom;

		// worldspace for the circle center
		vec2 pos = vec2(0, 0); 

		// convert worldspace to between -1 and 1.
		vec2 ss = (((pos - screen_min)/viewport_wh) * 2.0) - 1.0;
		ss.x *= aspect_x;
		vec2 p = tmp_uv + ss;

		float tilesize = 32;
		float tiles = 20;
		float radius = ((tilesize * tiles) / viewport_wh.y) * 2;
		d = sdfCircle(p, radius);
	}

	vec2 uv = v_uv - 0.5;
	uv *= zoom;
	vec2 cam_uv = camera_pos + (uv * viewport_wh);
	cam_uv /= viewport_wh;
	vec2 grid_uv = cam_uv;
	vec2 grid_p = ( viewport_wh / 1700 ) * grid_uv;

  float t = fworley(grid_p);	

	vec2 tex_uv = v_uv; // raw texture uv
	tex_uv.y = 1 - tex_uv.y;

	// inside distances only
	d = clamp(d, -1, 1); 

	// Multiply intensity values by a colour curve based off the uv
	// t *= exp(-length2(abs(0.7*tex_uv - 1.0))); // add gradient
	t *= exp(-length2(abs(0.7*vec2(0.60) - 1.0)));	

	vec3 col = vec3(0.0);

	vec3 water_col = vec3(
		 0.05,
		 min(1.1*t, 1.0),
		 min(pow(t, 0.75-t), 1.0) // colour curve
	);

	vec3 danger_col = vec3(
		 min(pow(t, 1.0-t), 1.0), // colour curve
		 min(1.1*t, 1.0),
		 0.05
	);

	// vec3[5] ocean = vec3[5](
  //   vec3(0/255.0, 26/255.0, 51/255.0),
  //   vec3(0/255.0, 51/255.0, 102/255.0),
  //   vec3(0/255.0, 64/255.0, 128/255.0),
  //   vec3(0/255.0, 89/255.0, 179/255.0),
  //   vec3(0/255.0, 102/255.0, 204/255.0)
  // );
	// if(!in_main_menu)
	{
		vec3 w_col = vec3(0/255.0, 64/255.0, 128/255.0);
		vec3 c = mix(danger_col, w_col, float(d < 0));
		c *= 1.0 - exp(-6.0*abs(d)); // dark edges
		out_colour.rgb = c;
		out_colour.a = 1.0f;
		return;
	}

	col = sqrt(sqrt(t)) * mix(danger_col, water_col, float(d < 0));	
	col *= 1.0 - exp(-6.0*abs(d)); // dark edges

	out_colour.rgb = col;

	// vec3 col = (d>0.0) ? vec3(0.9,0.6,0.3) : vec3(0.65,0.85,1.0);
	// out_colour.rgb = col;

	// use the fluid sim as a mask for the water shader.
	// vec3 fluid_c = texture(tex_fluid_sim, v_uv).rgb;
	// if(length(fluid_c) > 0.0)
	// 	out_colour.rgb = col;
	// else
	// 	out_colour.rgb = vec3(0.0);

	out_colour.a = 1.0f;
}

