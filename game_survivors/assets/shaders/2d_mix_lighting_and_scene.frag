// version prepended to file when loaded by engine.
//

out vec4 out_color;

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

uniform sampler2D tex_scene_0;         // linear main
uniform sampler2D tex_unit_water;
uniform sampler2D tex_outline;
uniform vec2 viewport_wh;
uniform bool add_grid;

layout(std140) uniform Data {
  mat4 projection_zoomed;
  mat4 view;
  vec2 camera_pos;
  float time;
  float zoom;
  float tilesize;
};

#define NR_MAX_CIRCLES 100

struct Light
{
    bool enabled;
    vec2 position;
    vec4 colour;
    float luminance;

    // not yet used
    // float range;
    // float radius;
};
#define MAX_LIGHTS 32
uniform Light lights[MAX_LIGHTS];

float
SRGBFloatToLinearFloat(const float f)
{
  if (f <= 0.04045f)
    return f / 12.92f;
  return pow((f + 0.055f) / 1.055f, 2.4f);
}

vec3 srgb_to_lin(vec3 color)
{
  vec3 result;
  result.x = SRGBFloatToLinearFloat(color.r / 255.0f);
  result.y = SRGBFloatToLinearFloat(color.g / 255.0f);
  result.z = SRGBFloatToLinearFloat(color.b / 255.0f);
  return result;
}

float
linear_to_srgb(float f)
{
  if (f <= 0.0031308f)
    return 12.92f * f;
  return 1.055f * pow(f, 1.0f / 2.4f) - 0.055f;
}

vec3 lin_to_srgb(vec3 color)
{
  vec3 x = color.rgb * 12.92;
  vec3 y = 1.055 * pow(clamp(color.rgb, 0.0, 1.0), vec3(0.4166667)) - 0.055;
  vec3 clr = color.rgb;
  clr.r = (color.r < 0.0031308) ? x.r : y.r;
  clr.g = (color.g < 0.0031308) ? x.g : y.g;
  clr.b = (color.b < 0.0031308) ? x.b : y.b;
  return clr.rgb;
}

// sdf

float circleDist(vec2 p, float radius)
{
    return length(p) - radius;
}

float sdGrid(in vec2 position, in float margin) {

    // Calculate per-axis distance from 0.5 to position mod 1
    vec2 gridDist = abs(fract(position) - 0.5) - margin;
    
    // Calculate length for round outer corners, by Inigo Quilez
    float outsideDist = length(max(gridDist, 0.0));
    // Calculate inside separately, by Inigo Quilez
    float insideDist = min(max(gridDist.x, gridDist.y), 0.0);
    
    return outsideDist + insideDist;
}

// https://www.shadertoy.com/view/3ltSW2
float sdCircle( vec2 p, float radius ) 
{
    return length(p) - radius;
}

// https://www.shadertoy.com/view/7sdXz2
// s = side length
// r = corner radius
float sdRoundSquare( in vec2 p, in float s, in float r ) 
{
    vec2 q = abs(p)-s+r;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r;
}

// https://www.shadertoy.com/view/lt3BW2
float opSmoothUnion( float d1, float d2, float k )
{
    float h = max(k-abs(d1-d2),0.0);
    return min(d1, d2) - h*h*0.25/k;
}

//
// blending modes...
//

vec3 multiply(in vec3 src, in vec3 dst)
{
    return src * dst;
}

vec3 screen(in vec3 src, in vec3 dst)
{
    return src + dst - src * dst;
}

vec3 overlay(in vec3 src, in vec3 dst)
{
    return mix(2.0 * src * dst, 1.0 - 2.0 * (1.0 - src) * (1.0-dst), step(0.5, dst));
}

vec3 hardlight(in vec3 src, in vec3 dst)
{
    return mix(2.0 * src * dst,  1.0 - 2.0 * (1.0 - src) * (1.0-dst), step(0.5, src));
}

vec3 softlight(in vec3 src, in vec3 dst)
{
    return mix(dst - (1.0 - 2.0 * src) * dst * (1.0 - dst), 
               mix(dst + ( 2.0 * src - 1.0 ) * (sqrt(dst) - dst),
                   dst + (2.0 * src - 1.0) * dst * ((16.0 * dst - 12.0) * dst + 3.0),
                   step(0.5, src) * (1.0 - step(0.25, dst))),
               step(0.5, src));
}

vec3 colorDodge(in vec3 src, in vec3 dst)
{
    return step(0.0, dst) * mix(min(vec3(1.0), dst/ (1.0 - src)), vec3(1.0), step(1.0, src)); 
}

vec3 colorBurn(in vec3 src, in vec3 dst)
{
    return mix(step(0.0, src) * (1.0 - min(vec3(1.0), (1.0 - dst) / src)),
        vec3(1.0), step(1.0, dst));
}

vec3 linearDodge(in vec3 src, in vec3 dst)
{
    return clamp(src.xyz + dst.xyz, 0.0, 1.0);
}

vec3 linearBurn(in vec3 src, in vec3 dst)
{
    return clamp(src.xyz + dst.xyz - 1.0, 0.0, 1.0);
}

vec3 vividLight(in vec3 src, in vec3 dst)
{
    return mix(max(vec3(0.0), 1.0 - min(vec3(1.0), (1.0 - dst) / (2.0 * src))),
               min(vec3(1.0), dst / (2.0 * (1.0 - src))),
               step(0.5, src));
}

vec3 linearLight(in vec3 src, in vec3 dst)
{
    return clamp(2.0 * src + dst - 1.0, 0.0, 1.0);;
}

vec3 pinLight(in vec3 src, in vec3 dst)
{
    return mix(mix(2.0 * src, dst, step(0.5 * dst, src)),
        max(vec3(0.0), 2.0 * src - 1.0), 
        step(dst, (2.0 * src - 1.0))
    );
}

vec3 hardMix(in vec3 src, in vec3 dst)
{
    return step(1.0, src + dst);
}

vec3 subtract(in vec3 src, in vec3 dst)
{
    return dst - src;
}

vec3 divide(in vec3 src, in vec3 dst)
{
    return dst / src;
}

vec3 addition(vec3 src, vec3 dst)
{
    return src + dst;
}

vec3 difference(in vec3 src, in vec3 dst )
{
    return abs(dst - src);   
}

vec3 darken(in vec3 src, in vec3 dst)
{
    return min(src, dst);
}

vec3 lighten(in vec3 src, in vec3 dst)
{
    return max(src, dst);
}

vec3 invert(in vec3 src, in vec3 dst)
{
    return 1.0 - dst;
}

vec3 invertRGB(in vec3 src, in vec3 dst)
{
    return src * (1.0 - dst);
}

vec3 source(in vec3 src, in vec3 dst)
{
    return src;
}

vec3 dest(in vec3 src, in vec3 dst)
{
    return dst;
}

// Branchless RGB2HSL implementation from : https://www.shadertoy.com/view/MsKGRW
vec3 rgb2hsl( in vec3 c )
{
    const float epsilon = 0.00000001;
    float cmin = min( c.r, min( c.g, c.b ) );
    float cmax = max( c.r, max( c.g, c.b ) );
    float cd   = cmax - cmin;
    vec3 hsl = vec3(0.0);
    hsl.z = (cmax + cmin) / 2.0;
    hsl.y = mix(cd / (cmax + cmin + epsilon), cd / (epsilon + 2.0 - (cmax + cmin)), step(0.5, hsl.z));

    vec3 a = vec3(1.0 - step(epsilon, abs(cmax - c)));
    a = mix(vec3(a.x, 0.0, a.z), a, step(0.5, 2.0 - a.x - a.y));
    a = mix(vec3(a.x, a.y, 0.0), a, step(0.5, 2.0 - a.x - a.z));
    a = mix(vec3(a.x, a.y, 0.0), a, step(0.5, 2.0 - a.y - a.z));
    
    hsl.x = dot( vec3(0.0, 2.0, 4.0) + ((c.gbr - c.brg) / (epsilon + cd)), a );
    hsl.x = (hsl.x + (1.0 - step(0.0, hsl.x) ) * 6.0 ) / 6.0;
    return hsl;
}

// HSL2RGB thanks to IQ : https://www.shadertoy.com/view/lsS3Wc
vec3 hsl2rgb(in vec3 c)
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

vec3 hue(in vec3 src, in vec3 dst)
{
    vec3 dstHSL = rgb2hsl(dst);
    vec3 srcHSL = rgb2hsl(src);
    return hsl2rgb(vec3(srcHSL.r, dstHSL.gb));
}

vec3 saturation(in vec3 src, in vec3 dst)
{
    vec3 dstHSL = rgb2hsl(dst);
    vec3 srcHSL = rgb2hsl(src);
    return hsl2rgb(vec3(dstHSL.r, srcHSL.g, dstHSL.b));
}

vec3 color(in vec3 src, in vec3 dst)
{
    vec3 dstHSL = rgb2hsl(dst);
    vec3 srcHSL = rgb2hsl(src);
    return hsl2rgb(vec3(srcHSL.rg, dstHSL.b));
}

vec3 luminosity(in vec3 src, in vec3 dst)
{
    vec3 dstHSL = rgb2hsl(dst);
    vec3 srcHSL = rgb2hsl(src);
    return hsl2rgb(vec3(dstHSL.rg, srcHSL.b));
}

vec3 Tonemap_ACES(const vec3 x) {
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

void main()
{
vec2 v_uv = fs_in.v_uv;
  vec4 v_colour= fs_in.v_colour;
  vec2 v_sprite_pos = fs_in.v_sprite_pos;
  vec2 v_sprite_wh = fs_in.v_sprite_wh;
  vec2 v_sprite_max = fs_in.v_sprite_max;
  int index = int(fs_in.v_tex_unit);

  out_color.a = 1.0f;
    
  // fragCoord : is a vec2 that is between 0 > 640 on the X axis and 0 > 360 on the Y axis
  // iResolution : is a vec2 with an X value of 640 and a Y value of 360
  vec2 fragCoord = (v_uv * viewport_wh);
  vec2 iResolution = viewport_wh;
    vec2 center = iResolution.xy * 0.5;
    vec2 p = ((fragCoord - center) * zoom + center + vec2(0.5));

    vec2 half_wh = viewport_wh / 2.0;
    vec2 screen_min = camera_pos - half_wh; // e.g. -960
    
    // sdf grid	
    vec3 grid_col = vec3(0.0f);
    if(add_grid) {
        {
            float gridsize = tilesize / zoom; // pixels
            vec2 camera_uv_screen = vec2( camera_pos.x / half_wh.x, camera_pos.y / half_wh.y); // camera position is in worldspace.
            vec2 camera_uv = camera_uv_screen / zoom; 
            float aspect_y = viewport_wh.y / viewport_wh.x;
            
            vec2 grid_uv = (2.0 * v_uv - 1.0);
            grid_uv += camera_uv;
            grid_uv.y *= aspect_y;

            vec2 p_grid = (viewport_wh.x / gridsize / 2.0) * grid_uv;

            // if the gridsize gets too small and the gridwidth isnt large enough, 
            // the grid appears to dissapear. the value 0.05 seems to work until gridsize<10
            // float grid_width = 0.02; 
            float grid_width = 0.04; 
            float margin = 0.5;
            if(abs(sdGrid(p_grid, margin)) >= grid_width)
                grid_col = vec3(0.0);// background
            else
                grid_col = vec3(0.04); // line
        }
    }

    vec4 scene_lin = texture(tex_scene_0, v_uv);
    vec4 outline_col = texture(tex_outline, v_uv);

    vec3 srgb_final = lin_to_srgb(scene_lin.rgb);
    vec3 srgb_water = texture(tex_unit_water, v_uv).rgb;

    if (length(scene_lin.rgb) > 0.0) {
        out_color.rgb = srgb_final;
    } else {
        out_color.rgb = srgb_water;
    }
    
    // ACES tonemap
    // out_color.rgb = Tonemap_ACES(out_color.rgb);

    if(outline_col.r > 0.0f)
        out_color.rgb = vec3(1.0, 1.0, 1.0);

    out_color.rgb += grid_col;

    // vignette
    // vec2 vig_uv = fragCoord.xy / iResolution.xy;
    // vig_uv *=  1.0 - vig_uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
    // float vig = vig_uv.x*vig_uv.y * 15.0; // multiply with sth for intensity
    // vig = pow(vig, 0.15); // change pow for modifying the extend of the  vignettea
    // out_color.rgb *= vig;

    out_color.a = 1.0f;
}