// version prepended to file when loaded by engine.
//

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D scene_texture;
uniform sampler2D blur_texture;
uniform float exposure;
uniform bool do_bloom;

void
main()
{
  // work out "bright" areas for bloom effect
  // float brightness = luminance(out_color);
  // vec4 bright_colour = vec4(0.0, 0.0, 0.0, 1.0);
  // if(brightness > brightness_threshold)
  //   bright_colour = vec4(out_color.rgb, 1.0);
  // out_bright_color = bright_colour;

  // // after lighting, clamp buffer
	// out_color.rgb = clamp(out_color.rgb, 0.0, 1.0);
  // out_color.a = 1.0f;

  // color = sprite_colour; //* texture(tex, TexCoords);
  // color = sprite_colour * texture(tex, TexCoords);
  vec4 scene_col_srgb = texture(scene_texture, v_uv);
  vec4 blur_col_srgb = texture(blur_texture, v_uv);
  
  if(do_bloom){
    vec4 blend = scene_col_srgb + blur_col_srgb;

    // tone mapping
    vec3 result = vec3(1.0) - exp(-blend.rgb * exposure);

    out_color = vec4(result, 1.0);
  } else {
    out_color = scene_col_srgb;
  }

}