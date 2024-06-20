#version 460

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D tex;
uniform float brightness_threshold;

void
main()
{
  vec4 scene_col = texture(tex, v_uv);

  // work out "bright" areas
  float brightness = dot(scene_col.rgb, vec3(0.2126, 0.7152, 0.0722));
  vec4 bright_colour = vec4(0.0, 0.0, 0.0, 1.0);
  if(brightness > brightness_threshold)
    bright_colour = vec4(scene_col.rgb, 1.0);
  
  out_color = bright_colour;
}