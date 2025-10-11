// version prepended to file when loaded by engine.
//

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

uniform sampler2D tex_island_triangles;
uniform vec2 screen_wh;

vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
				   dot(p,vec2(269.5,183.3)), 
				   dot(p,vec2(419.2,371.9)) );
	return fract(sin(q)*43758.5453);
}

float voronoise( in vec2 p, float u, float v )
{
	float k = 1.0+63.0*pow(1.0-v,6.0);

    vec2 i = floor(p);
    vec2 f = fract(p);
    
	vec2 a = vec2(0.0,0.0);
    for( int y=-2; y<=2; y++ )
    for( int x=-2; x<=2; x++ )
    {
        vec2  g = vec2( x, y );
		vec3  o = hash3( i + g )*vec3(u,u,1.0);
		vec2  d = g - f + o.xy;
		float w = pow( 1.0-smoothstep(0.0,1.414,length(d)), k );
		a += vec2(o.z*w,w);
    }
	
    return a.x/a.y;
}

vec3 truncRound( vec3 a, float l )
{
	return floor(a*l+0.5)/l;
}

void
main()
{
  vec2 v_uv = fs_in.v_uv;
  vec4 v_colour = fs_in.v_colour;

  vec3 tex_col = texture2D(tex_island_triangles, v_uv).rgb;

	vec2 cam_uv = camera_pos + (v_uv * screen_wh);
  cam_uv /= screen_wh;

  // out_colour.rg = cam_uv;
  // out_colour.a = 1.0;

	// vec2 cam_uv = camera_pos + (uv * screen_wh);
	// cam_uv /= screen_wh;

  vec2 p = vec2(1.0);
  // float n = voronoise(20 * cam_uv, p.x, p.y);
  int levels = 11;
  // vec3 col = tex_col * vec3(n, n, n) * vec3(0.4, 1.0, 0.8);

  float n = 0.5f;
  vec3 col = tex_col * vec3(n, n, n) * vec3(0.4, 1.0, 0.8);
  vec3 q_col = truncRound( col , levels );
  out_colour.rgb = q_col;

  // out_colour.rgb = tex_col * vec3(length(abs(cam_uv.x)), length(abs(cam_uv.y)), 0.0);

  out_colour.a = 1.0f;
}