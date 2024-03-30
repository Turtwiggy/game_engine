#version 460 core

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprites;    // amount of sprites (x, y)
in float v_tex_unit;

uniform sampler2D tex;

void
main()
{
  out_colour = vec4(v_uv.x, v_uv.y, 0.0, 1.0);
  // out_colour = v_colour;

  // // Sample texture directly
  // if (v_sprite_pos.x == 0.0f && v_sprite_pos.y == 0.0f) { // a whole texture
  //   return;
  // } 

  // // A spritesheet texture
  // if(v_sprites.x > 0.0f || v_sprites.y > 0.0f)
  // {
  //   float scale_x = 1.0f / v_sprites.x;
  //   float scale_y = 1.0f / v_sprites.y;

  //   vec2 sprite_uv = vec2(
  //     v_uv.x / v_sprites.x + v_sprite_pos.x * scale_x,
  //     v_uv.y / v_sprites.y + v_sprite_pos.y * scale_y      
  //   );

  //   out_colour = v_colour;
  //   out_colour *= texture(tex, sprite_uv);
  // }
}

// if (do_lighting) {
//   // non-pixel, directly sample texture
//   vec2 uv = v_uv;
//   uv = uv_cstantos(uv, vec2(screen_w, screen_h));
//   vec4 tex_main = texture(textures[1], uv);
//   vec4 tex_shadow = texture(textures[2], uv);
//   vec4 r;
//   for (int i = 0; i < num_lights; i++) {
//     if (light_enabled[i]) {
//       float distance = length(light_pos[i] - FragPos);
//       const float light_constant = 1.0f;
//       float linear = light_linear[i];
//       float quadratic = light_quadratic[i];
//       float attenuation = 1.0 / (light_constant + linear * distance + quadratic * (distance * distance));
//       vec4 c = tex_main * attenuation;
//       r += c;
//     }
//   }
//   vec4 c = r;
//   // vec4 c = tex_main;
//   // pixel is in shadow
//   if (tex_shadow.r == 0.0f) {
//     c = c * vec4(0.92f, 0.92f, 0.92f, 1.0f);
//   }
//   out_colour = c;
//   out_colour.a = 1.0f;
//   return;
// }

// uniform bool do_lighting = false;
// const int num_lights = 32;
// uniform bool light_enabled[num_lights];
// uniform vec3 light_pos[num_lights];
// uniform float light_linear[num_lights];
// uniform float light_quadratic[num_lights];

