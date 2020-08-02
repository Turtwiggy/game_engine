#version 430 core

layout(binding = 0, rgba16f) writeonly uniform image2D outTexture;
layout(binding = 1, rgba16f) readonly uniform image2D normalTexture;

struct vertex {
    vec4 pos;
    vec4 normal;
    vec4 tex;
};

struct triangle {
    vertex p0;
    vertex p1;
    vertex p2;
};

uniform int set_triangles;

layout( std430, binding = 2 ) readonly buffer bufferData
{
    triangle triangles[];
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
#define MAX_SCENE_BOUNDS 100.0

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
vec3 trace(ray r, vec3 normal) {

    vec3 origin = r.origin;
    vec3 att = vec3(1.0);

    const float bounces = 1;

    for(int bounce = 0; bounce < bounces; bounce++) {

        //use texture information for first bounce
        if(bounce == 0 && dot(r.direction, normal) == 0.0) {
            //did not hit anything!
            return LIGHT_INTENSITY * SKY_COLOUR * att;
        }

        // the ray hit something!
        vec3 visulized_normal = 0.5 * vec3(normal.x + 1, normal.y + 1, normal.z + 1);
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


layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {

    ivec2 px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outTexture);

    if (any(greaterThanEqual(px, size)))
        return;

    vec2 p = (vec2(px) + vec2(0.5)) / vec2(size);
    vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);
    ray fwd = {eye, dir};

    //Sample textures
    //vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalTexture, px).rgb;

    vec3 color = trace(fwd, Normal);

    imageStore(outTexture, px, vec4(color, 1.0));
}
