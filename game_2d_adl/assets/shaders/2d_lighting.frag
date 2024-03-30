#version 460 core

out vec4 out_colour;

in vec2 v_uv;
in vec4 v_colour;
in vec2 v_sprite_pos; // x, y location of sprite
in vec2 v_sprite_wh;  // desired sprites e.g. 2, 2
in vec2 v_sprite_max; // 22 sprites
in float v_tex_unit;

struct LineSegment {
    vec2 p0;
    vec2 p1;
    vec3 emissiveColor;
};
struct Point{
    vec2 p;
    vec3 emissiveColor;
};

const float PI = 3.14159265;

const int max_points = 50;
const int max_lines = max_points/2;

float angles[2 * max_lines];
LineSegment[max_lines] segments;

uniform vec2 viewport_wh;
uniform int active_lines;
uniform Point[max_points] points;


void sortAngles() {
    for (int i = 0; i < active_lines; ++i) {
        for (int j = 0; j < 2; ++j) {
            int k = 2 * i + j;
            vec2 p = j == 0 ? segments[i].p0 : segments[i].p1;
            float angle = mod(atan(p.y, p.x), 2.0 * PI);
            int l = k - 1;
            
            while (l >= 0 && angle < angles[l]) {
                angles[l + 1] = angles[l];
                l -= 1;
            }
            
            angles[l + 1] = angle;
        }
    }
}

vec3 integrateRadiance(LineSegment a, vec2 angle) {
    return (angle[1] - angle[0]) * a.emissiveColor;
}

vec3 integrateSkyRadiance_(vec2 angle) {
    float a1 = angle[1];
    float a0 = angle[0];
    
    // https://www.shadertoy.com/view/NttSW7
    const vec3 SkyColor = vec3(0.2,0.5,1.);
    const vec3 SunColor = vec3(1.,0.7,0.1)*10.;
    const float SunA = 2.0;
    const float SunS = 64.0;
    const float SSunS = sqrt(SunS);
    const float ISSunS = 1./SSunS;
    vec3 SI = SkyColor*(a1-a0-0.5*(cos(a1)-cos(a0)));
    SI += SunColor*(atan(SSunS*(SunA-a0))-atan(SSunS*(SunA-a1)))*ISSunS;
    return SI / 6.0;
}

vec3 integrateSkyRadiance(vec2 angle) {
    if (angle[1] < 2.0 * PI) {
        return integrateSkyRadiance_(angle);
    }
    
    return integrateSkyRadiance_(vec2(angle[0], 2.0 * PI)) + integrateSkyRadiance_(vec2(0.0, angle[1] - 2.0 * PI));
}

int findIndex(float angle) {
    mat2 m;
    m[1] = vec2(cos(angle), sin(angle));
    int bestIndex = -1;
    float bestU = 1e10;
    
    for (int i = 0; i < active_lines; ++i) {
        m[0] = segments[i].p0 - segments[i].p1;
        vec2 tu = inverse(m) * segments[i].p0;
        if (tu == clamp(tu, vec2(0.0), vec2(1.0, bestU))) {
            bestU = tu.y;
            bestIndex = i;
        }
    }
    
    return bestIndex;
}

vec3 calculateFluence() {
    vec3 fluence = vec3(0.0);
    
    for (int i = 0; i < 2 * active_lines; ++i) {
        vec2 a;
        a[0] = angles[i];
        
        if (i + 1 < 2 * active_lines) {
            a[1] = angles[i + 1];
        } else {
            a[1] = angles[0] + 2.0 * PI;
        }
        
        if (a[0] == a[1]) {
            continue;
        }
        
        int j = findIndex((a[0] + a[1]) / 2.0);
        
        if (j == -1) {
            fluence += integrateSkyRadiance(a);
        } else {
            fluence += integrateRadiance(segments[j], a);
        }
    }
    
    return fluence;
}

float sdf(LineSegment l, vec2 p) {
    vec2 pa = p-l.p0, ba = l.p1-l.p0;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

vec4 blendOver(vec4 top, vec4 bottom) {
    float a = top.a + bottom.a * (1.0 - top.a);
    return vec4((top.rgb * top.a + bottom.rgb * bottom.a * (1.0 - top.a)) / a , a);
}

void drawSDF(inout vec4 dst, vec4 src, float sdf) {
    dst = blendOver(vec4(src.rgb, src.a * clamp(1.5 - abs(sdf), 0.0, 1.0)), dst);
}

void
main()
{
    const vec2 fragCoord = v_uv * viewport_wh;
    for (int i = 0; i < active_lines; ++i) {
        segments[i].p0 = (points[2 * i + 0].p.xy * viewport_wh.xy) - fragCoord;
        segments[i].p1 = (points[2 * i + 1].p.xy * viewport_wh.xy) - fragCoord;
        segments[i].emissiveColor = points[2 * i + 0].emissiveColor; // on point a
    }

  sortAngles();
  const vec3 fluence = calculateFluence();

  out_colour = vec4(1.0 - 1.0 / pow(1.0 + fluence, vec3(3.0)), 1.0);
  out_colour.a = 1.0;

  for (int i = 0; i < active_lines; ++i) {
    drawSDF(
        out_colour, 
        vec4(3.0 * pow(segments[i].emissiveColor, vec3(1.0 / 2.2)), 1.0),
        sdf(segments[i], vec2(0.0))
    );
  }
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

