// version prepended to file when loaded by engine.
//

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform vec2 viewport_wh;
uniform vec2 player_position;

//
// Star Nest by Pablo Roman Andrioli
// License: MIT
// https://www.shadertoy.com/view/XlfGRj
//

#define iterations 15
#define formuparam 0.53

#define volsteps 10
#define stepsize 0.2

#define zoom   0.750
#define tile   0.850

#define brightness 0.0008
#define darkmatter 0.800
#define distfading 0.730
#define saturation 0.950

void main()
{
  vec2 fragCoord = v_uv * viewport_wh;
  vec2 iResolution = viewport_wh;

	//get coords and direction
	vec2 uv=fragCoord.xy/iResolution.xy-.5;
	uv.y*=iResolution.y/iResolution.x;
	vec3 dir=vec3(uv*zoom,1.);

	vec3 from = vec3(player_position.x / 10000.0f, player_position.y / 10000.0f, 0.5f);
	
	//volumetric rendering
	float s=0.1,fade=1.;
	vec3 v=vec3(0.);
	for (int r=0; r<volsteps; r++) {
		vec3 p=from+s*dir*.5;
		p = abs(vec3(tile)-mod(p,vec3(tile*2.))); // tiling fold
		float pa,a=pa=0.;
		for (int i=0; i<iterations; i++) { 
			
			float p_dot = dot(p,p);

			// reduce flicking, but also detail. hmmmmmm.
			// the lower the number, the less flickering.
			if(p_dot < 0.001) {
				p_dot = 0.001;
			}

			p=abs(p)/p_dot-formuparam; // the magic formula
			a+=abs(length(p)-pa); // absolute sum of average change
			pa=length(p);
		}
		float dm=max(0.,darkmatter-a*a*.001); //dark matter
		a*=a*a; // add contrast
		if (r>6) fade*=1.-dm; // dark matter, don't render near
		//v+=vec3(dm,dm*.5,0.);
		v+=fade;
		v+=vec3(s,s*s,s*s*s*s)*a*brightness*fade; // coloring based on distance
		fade*=distfading; // distance fading
		s+=stepsize;
	}
	
	v=mix(vec3(length(v)),v,saturation); //color adjust
  
	out_colour = vec4(v*.01,1.);	
}