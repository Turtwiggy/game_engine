#version 430 core

layout(binding = 0, rgba16f) uniform image2D positionData;
layout(binding = 1, rgba16f) uniform image2D normalData;
layout(binding = 2, rgba16f) uniform image2D albedoSpecData;
layout(binding = 3, rgba32f) uniform image2D outTexture;

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
float random(vec3 f);
vec3 randomHemispherePoint(vec3 n, vec2 rand);

struct ray {
    vec3 origin, direction ;
};

vec3 ray_at(const ray r, float t) {
    return r.origin + (t * r.direction);
}

struct hitinfo {
    vec3 point;
    vec3 normal;
    float t;
};

struct sphere {
    vec3 center;
    float radius;
};

bool hit_sphere(const sphere s, const ray r, float t_min, float t_max, out hitinfo hit)
{
    vec3 oc = r.origin - s.center;
    float a = dot(r.direction, r.direction);
    float half_b = dot(oc, r.direction);
    float c = dot(oc, oc) - s.radius*s.radius;
    float discriminant = half_b*half_b - a*c;

    if (discriminant > 0) {
        //got a hit!

        float root = sqrt(discriminant);

        float temp = (-half_b - root) / a;
        if (temp < t_max && temp > t_min) {
            hit.t = temp;
            hit.point = ray_at(r, hit.t);
            hit.normal = (hit.point - s.center) / s.radius;
            return true;
        }

        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min) {
            hit.t = temp;
            hit.point = ray_at(r, hit.t);
            hit.normal = (hit.point - s.center) / s.radius;
            return true;
        }
    }
    return false;
}

//https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
vec3 trace(ray r, vec3 diffuse, vec3 frag_pos, vec3 normal) {

    //sphere s = { vec3(0.0, 0.0, -1.0), 0.5f };
    //hitinfo hinfo;

    //if(hit_sphere(s, r, 0, 1, hinfo)) {
    //    vec3 N = normalize(hinfo.point);
    //    return 0.5* vec3(N.x + 1.0, N.y + 1, N.z + 1);
    //}

    // if ( dot(r.direction, normal) > 0.0) {
    //     //frontface = false;
    //     return visulized_normal;
    // }
    // //ray is from outside the normal
    // if( dot(r.direction, normal) < 0.0) {
    //     //frontface = true;
    //     return visulized_normal;
    // }

    vec3 origin = r.origin;
    vec3 att = vec3(1.0);
    const float bounces = 1;

    for(int bounce = 0; bounce < bounces; bounce++) {

        if(dot(r.direction, normal) == 0.0){
            //did not hit anything!
            return LIGHT_INTENSITY * SKY_COLOUR * att;
        }

        // the ray hit something!
        vec3 visulized_normal = 0.5 * vec3(frag_pos.x + 1, frag_pos.y + 1, frag_pos.z + 1);
        return visulized_normal;

        //get the point of intersection
        //vec3 point = origin + hinfo.near * dir;
        //vec3 normal = hinfo.normal;

        //offset point small amount by surface normal
        //origin = point + normal * EPSILON;

        //evaluate the surface BRDF
        //vec3 dir = randomHemispherePoint(normal, randvec2(bounce));

        //att *= ONE_OVER_PI;
        //att *= dot(dir, normal);

        //colour of material (albedo)
        //att *= b.col;   
        //att *= vec3(1.0, 0.1, 0.1);
        //att /= ONE_OVER_2PI;
    }

    // bool frontface;
    // //ray is from inside normal
    return vec3(0.0);
}

//GLOBALS
ivec2 px;

layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {

    px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(albedoSpecData);

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
    vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalData, px).rgb;
    vec3 Diffuse = imageLoad(albedoSpecData, px).rgb;
    float Specular = imageLoad(albedoSpecData, px).a;

    vec3 color = trace(fwd, Diffuse, FragPos, Normal);

    imageStore(outTexture, px, vec4(color, 1.0));
}
