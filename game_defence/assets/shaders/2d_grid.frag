#version 460

out vec4 o_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform vec2 viewport_wh;
uniform vec2 camera_pos;
uniform float gridsize;

float sdGrid(in vec2 position, in float margin) {

	// Calculate per-axis distance from 0.5 to position mod 1
	vec2 gridDist = abs(fract(position) - 0.5) - margin;
	
	// Calculate length for round outer corners, ripped from Inigo Quilez
	float outsideDist = length(max(gridDist, 0.0));
	// Calculate inside separately, ripped from Inigo Quilez
	float insideDist = min(max(gridDist.x, gridDist.y), 0.0);
	
	return outsideDist + insideDist;
}

vec2 offset(vec2 _st, vec2 _offset){
  vec2 uv;
  if(_st.x>0.5){
      uv.x = _st.x - 0.5;
  } else {
      uv.x = _st.x + 0.5;
  }
  if(_st.y>0.5){
      uv.y = _st.y - 0.5;
  } else {
      uv.y = _st.y + 0.5;
  }
  return uv;
}

void
main()
{
  const vec2 half_wh = viewport_wh / 2.0;
  const float screen_min_x = camera_pos.x - half_wh.x; // e.g. -960
  const float screen_max_x = camera_pos.x + half_wh.x; // e.g. 960
  const float screen_min_y = camera_pos.y - half_wh.y; // e.g. -540
  const float screen_max_y = camera_pos.y + half_wh.y; // e.g. 540
  const float range_x = screen_max_x - screen_min_x;
  const float range_y = screen_max_y - screen_min_y;

  //
  // the screen overlay quad is moving with camera_position..
  // the uv is consistent for that.
  //

  // camera position is in worldspace.
  //
  const vec2 camera_uv_screen = vec2(
    camera_pos.x / half_wh.x,
    camera_pos.y / half_wh.y
  );
  const vec2 camera_uv = camera_uv_screen; 
  //
  // here, camera_uv_screen should be in the range 0, 1
  // e.g. 0 < camera_pos.x < half_wh.x
  //

  // convert uv to -1 and 1
  vec2 uv = 2.0 * v_uv - 1.0;

  // The grid-square that is moving around
  // also has uv-coordinates,
  uv += camera_uv;

  // fix for aspect
  // float aspect = viewport_wh.x / viewport_wh.y;
  float aspect = viewport_wh.y / viewport_wh.x;
  uv.y *= aspect;

  vec2 p = (viewport_wh.x / gridsize / 2.0) * uv;
  float margin = 0.5;
  float d = sdGrid(p, margin);

  // vec3 colour = vec3(1.0) - sign(d) * vec3(0.1, 0.4, 0.7);
  vec3 colour = vec3(1.0, 0.0, 0.0);

  // if the gridsize gets too small
  // and the gridwidth isnt large enough, 
  // the grid appears to dissapear.
  // the value 0.05 seems to work until gridsize<10
  const float grid_width = 0.02; 
  if(abs(d) >= grid_width)
    colour = vec3(0.0); // background
  else
    colour = vec3(0.1, 0.1, 0.1); // line

	// colour *= 1.0 - exp(-6.0 * abs(d));
	// colour *= 0.8 + 0.2 * cos(120.0 * d);
	// colour = mix(colour, vec3(1.0), 1.0 - smoothstep(0.0, 0.015, abs(d)));

  o_colour = vec4(colour, 1.0);
}