#version 430 core

layout(binding = 0, rgba16f) uniform image2D outTexture;

struct vertex {
    vec4 pos;
    vec4 normal;
    vec4 tex;
};

struct triangle {
    vec4 a;
    vec4 b;
    vec4 c;
};

layout(std430, binding = 1 ) readonly buffer bufferData
{
    vec4 triangles[];
};

uniform vec3 eye, ray00, ray01, ray10, ray11;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI (1.0 / PI)
#define ONE_OVER_2PI (1.0 / TWO_PI)
#define LARGE_FLOAT 1E+10
#define EPSILON 0.0001
#define LIGHT_INTENSITY 1.0
#define SKY_COLOUR vec3(0.2, 0.3, 1.0)

// Forward-declare external functions from random.glsl.
//float random(vec3 f);
//vec3 randomHemispherePoint(vec3 n, vec2 rand);

struct ray {
    vec3 origin, direction ;
};

vec3 ray_at(const ray r, float t) {
    return r.origin + (t * r.direction);
}

//GLOBALS
ivec2 px;

layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {

    px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outTexture);

    if (any(greaterThanEqual(px, size)))
        return;

    /*
       * Now we take our rayNN uniforms declared above to determine the
       * world-space direction from the eye position through the current
       * work item's pixel's center in the framebuffer image. We use the
       * 'px' variable, cast it to a floating-point vector, offset it by
       * half a pixel's width (in whole pixel units) and then transform that
       * position relative to our framebuffer size to get values in the
       * interval [(0, 0), (1, 1)] for all work items covering our
       * framebuffer.
   */
    vec2 p = (vec2(px) + vec2(0.5)) / vec2(size);

    /*
       * Use bilinear interpolation based on the X and Y fraction
       * (within 0..1) with our rayNN vectors defining the world-space
       * vectors along the corner edges of the camera's view frustum. The
       * result is the world-space direction of the ray originating from the
       * camera/eye center through the work item's framebuffer pixel center.
    */
    vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);

    ray fwd = {eye, dir};

    //Sample textures
    //vec3 FragPos = imageLoad(positionData, px).rgb;
    //vec3 Normal = imageLoad(normalData, px).rgb;
    //vec3 Diffuse = imageLoad(albedoSpecData, px).rgb;
    //float Specular = imageLoad(albedoSpecData, px).a;

    //vec3 color = trace(fwd, Normal);

    vec3 color = vec3(0.0, 0.0, 1.0);
    if(triangles.length() > 0)
    {
        //green = triangles
        color = vec3(0.2, 0.6, 0.2);
    } else
    {
        //red = bad
        color = vec3(0.6, 0.2, 0.2);
    }

    imageStore(outTexture, px, vec4(color, 1.0));
}
