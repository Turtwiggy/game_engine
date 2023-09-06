#version 460

out vec4 out_color;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;

void
main()
{
  // translate uvs from rectangular input texture to square voronoi texture.
	ivec2 tex_size = textureSize(tex, 0);
	vec2 uv = v_uv;
	if(tex_size.x > tex_size.y)
		uv.y = ((uv.y - 0.5) * (float(tex_size.x) / float(tex_size.y))) + 0.5;
	else
		uv.x = ((uv.x - 0.5) * (float(tex_size.y) / float(tex_size.x))) + 0.5;
		
  // any pixel with .a > 0 will be recognised as an emitter or occluder
  vec4 scene_col = texture(tex, uv);
  out_color = vec4(v_uv.x * scene_col.a, v_uv.y * scene_col.a, 0.0, 1.0);
}