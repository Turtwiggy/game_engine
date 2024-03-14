#version 460

out vec4 o_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

uniform float time;
uniform vec2 viewport_wh;
uniform vec2 camera_pos;

vec2 tile(vec2 uv, float amount){
  uv *= amount;
  return fract(uv);
}

float box(vec2 _st, vec2 _size, float _smoothEdges){
    _size = vec2(0.5)-_size*0.5;
    vec2 aa = vec2(_smoothEdges*0.5);
    vec2 uv = smoothstep(_size,_size+aa,_st);
    uv *= smoothstep(_size,_size+aa,vec2(1.0)-_st);
    return uv.x*uv.y;
}

// vec2 rotate2D(vec2 _st, float _angle){
//     _st -= 0.5;
//     _st =  mat2(cos(_angle),-sin(_angle),
//       sin(_angle),cos(_angle)) * _st;
//     _st += 0.5;
//     return _st;
// }

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
  vec2 half_wh = viewport_wh / 2.0;
  float screen_min_x = camera_pos.x - half_wh.x; // e.g. -960
  float screen_max_x = camera_pos.x + half_wh.x; // e.g. 960
  float screen_min_y = camera_pos.y - half_wh.y; // e.g. -540
  float screen_max_y = camera_pos.y + half_wh.y; // e.g. 540
  float range_x = screen_max_x - screen_min_x;
  float range_y = screen_max_y - screen_min_y;

  //
  // the screen overlay quad is moving with camera_position..
  // the uv is consistent for that.
  //

  // camera position is in worldspace.
  //
  vec2 camera_uv_screen = vec2(
    camera_pos.x / half_wh.x,
    camera_pos.y / half_wh.y
  );
  vec2 camera_uv = camera_uv_screen; 
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

  // if tiles are e.g. 64 pixels wide.......
  float tile_amount = float(viewport_wh.x / 128);

  vec2 grid_uv = tile(uv, tile_amount);
  vec3 colour = vec3(box(grid_uv, vec2(0.96), 0.0));
  // colour = vec3(1.0) - colour;

  if(colour == vec3(1.0, 1.0, 1.0)) // background
    colour = vec3(0.0, 0.0, 0.0);
  else if(colour == vec3(0.0, 0.0, 0.0)) // line
    colour = vec3(0.0075, 0.0075, 0.0075);

  o_colour = vec4(colour, 1.0);
  // o_colour = v_colour;
  // o_colour = vec4(0.0f, 0.5f, 0.0f, 1.0f);
}