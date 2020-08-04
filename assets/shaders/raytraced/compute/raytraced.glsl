#version 430 core

layout(binding = 0, rgba16f) writeonly uniform image2D outTexture;
layout(binding = 1, rgba16f) readonly uniform image2D normalTexture;

struct vertex {
    vec4 pos;
    vec4 normal;
    vec4 tex;
    vec4 colour;
};

struct triangle {
    vertex p0;
    vertex p1;
    vertex p2;
};


layout( std430, binding = 2 ) readonly buffer bufferData
{
    triangle triangles[];
};

//global
ivec2 px;
const vec3 lightCenterPosition = vec3(1.5, 1.5, 1.5);
const vec4 lightColor = vec4(1);

uniform int set_triangles;
uniform float time;
uniform vec3 eye, ray00, ray01, ray10, ray11;

uniform float phongExponent;
uniform float specularFactor;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI (1.0 / PI)
#define ONE_OVER_2PI (1.0 / TWO_PI)
#define LARGE_FLOAT 1E+10
#define EPSILON 0.0001
#define LIGHT_INTENSITY 1.0
#define SKY_COLOUR vec3(0.2, 0.3, 1.0)
#define MAX_SCENE_BOUNDS 100.0

// See random.glsl for more explanation of these functions.
float random(vec3 f);
vec4 randomHemispherePoint(vec3 n, vec2 rand);
float hemisphereProbability(vec3 n, vec3 v);
vec4 randomDiskPoint(vec3 n, float d, float r, vec2 rand);
float diskProbability(vec3 n, float d, float r, vec3 v);

vec3 randvec3(int s) {
  return vec3(
    random(vec3(px + ivec2(s), time)),
    random(vec3(px + ivec2(s), time * 1.1)),
    random(vec3(px + ivec2(s), time * 0.3)));
}

struct ray {
    vec3 origin, direction;
};

vec3 ray_at(const ray r, float t) {
    return r.origin + (t * r.direction);
}

struct hitinfo {
    vec3 point;
    vec3 normal;

    float u;
    float v;
    float t;

    int tri_index;
};

bool intersects_triangle(ray r, triangle tri, out hitinfo i)
{
    vec3 v0 = tri.p0.pos.xyz;
    vec3 v1 = tri.p1.pos.xyz;
    vec3 v2 = tri.p2.pos.xyz;
    vec3 dir = r.direction;
    vec3 orig = r.origin;

    vec3 v0v1 = v1-v0;
    vec3 v0v2 = v2-v0;

    vec3 pvec = cross(dir, v0v2);
    float det = dot(v0v1, pvec);

    //culling
    // if(det < EPSILON) {
    //    return false;
    // }
    //no culling
    if(abs(det) < EPSILON) {
        return false;
    }

    float inv_det = 1 / det;

    vec3 tvec = orig - v0;
    float u = dot(tvec, pvec) * inv_det;
    if( u < 0.0 || u > 1.0 ) { return false; }

    vec3 qvec = cross(tvec, v0v1);
    float v = dot(dir, qvec) * inv_det;
    if( v < 0.0 || u + v > 1.0) { return false; }

    float t = dot(v0v2, qvec) * inv_det;
    if( t > EPSILON)
    {
        //intersection point
        i.point = (orig + dir * t);

        //intersection normal
        i.normal = normalize(cross(v0v1, v0v2));

        //intersection distance
        i.t = t;

        i.u = u;
        i.v = v;

        return true;
    }

    return false; // this ray hits the triangle 
}

bool intersects_any_triangle(ray r, out hitinfo info) {

    float t_nearest = LARGE_FLOAT;
    bool intersect = false;
    int t_index;

    for(int i = 0; i < set_triangles; i++) {

        hitinfo h;

        const triangle tri = triangles[i];

        if(intersects_triangle(r, tri, h) && h.t < t_nearest)
        {
            //a closer triangle intersected the ray!
            intersect = true;

            t_nearest = h.t;
            t_index = i;
        }
    }

    info.tri_index = t_index;
    return intersect;
}

/**
 * Evaluate the specular part of the BRDF.
 *
 * @param b the box to evaluate (used to get its diffuse color)
 * @param i the incoming light direction
 *          (by convention this points away from the surface)
 * @param o the outgoing light direction
 * @param n the surface normal
 * @returns the attenuation factor
 */
vec3 brdfSpecular(vec3 i, vec3 o, vec3 n) {
  float a = phongExponent;
  vec3 r = reflect(-i, n);
  return vec3(pow(max(0.0, dot(r, o)), a) * (a + 2.0) * ONE_OVER_2PI);
}

/**
 * Evaluate the diffuse part of the BRDF.
 *
 * @param albedo the diffuse color
 * @param i the incoming light direction
 *          (by convention this points away from the surface)
 * @param o the outgoing light direction
 * @param n the surface normal
 * @returns the attenuation factor
 */
vec3 brdfDiffuse(vec3 albedo, vec3 i, vec3 o, vec3 n) {
  return albedo * ONE_OVER_PI;
}

/**
 * Compute the BRDF of the box's surface given the incoming and outgoing
 * light directions as well as the surface normal.
 *
 * @param albedo the diffuse color
 * @param i the incoming light direction
 *          (by convention this points away from the surface)
 * @param o the outgoing light direction
 * @param n the surface normal
 * @returns the attenuation factor
 */
vec3 brdf(vec3 albedo, vec3 i, vec3 o, vec3 n) {
  return brdfSpecular(i, o, n) * specularFactor
         +
         brdfDiffuse(albedo, i, o, n) * (1.0 - specularFactor);
}

//https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
vec3 trace(ray r, vec3 normal) {

    vec3 att = vec3(1.0);
    bool intersected = false;

    vec3 colour = vec3(0.0);
    ray next_ray = r;

    for(int bounce = 0; bounce < 2; bounce += 1 ) {

        hitinfo i;

        if(!intersects_any_triangle(next_ray, i))
        {
            return LIGHT_INTENSITY * SKY_COLOUR * att;
        }

        triangle tri = triangles[i.tri_index];

        vec3 albedo_colour = tri.p0.colour.xyz;
        vec3 point_of_intersection = i.point;
        vec3 normal_at_intersection = i.normal;

        //offset point of intersection
        point_of_intersection = i.point + i.normal * EPSILON;

        vec3 rand = randvec3(bounce);
        vec4 s = randomHemispherePoint(i.normal, rand.xy);

        att *= brdf(albedo_colour, s.xyz, - next_ray.direction, i.normal);

        ray new_ray;
        new_ray.origin = point_of_intersection;
        new_ray.direction = s.xyz;

        att *= max(0.0, dot( new_ray.direction, normal_at_intersection));

        if(s.w > 0.0)
            att /= s.w;

        next_ray = new_ray;

        return 0.5 * vec3( normal.x + 1,  normal.y + 1,  normal.z + 1);

        float n_dot_l = -dot(tri.p0.normal.xyz, next_ray.direction);

        return albedo_colour;
    }

    return LIGHT_INTENSITY * SKY_COLOUR * att;

    //return the normal colour
    //return 0.5 * vec3(intersection_point_normal.x + 1, intersection_point_normal.y + 1, intersection_point_normal.z + 1);
    //return the triangles's v0 colour!
    //return tri.p0.colour.xyz;
}



layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {

    px = ivec2(gl_GlobalInvocationID.xy);

    ivec2 size = imageSize(outTexture);

    if (any(greaterThanEqual(px, size)))
        return; // <- no work to do, return.

    vec2 p = (vec2(px) + vec2(0.5)) / vec2(size);
    vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);
    ray fwd = {eye, dir};

    //Sample textures
    //vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalTexture, px).rgb;

    //use texture information for first bounce
    //if(dot(dir, Normal) == 0.0) {  //no intersection
    //    imageStore(outTexture, px, vec4(SKY_COLOUR, 1.0));
    //    return;
    //}

    //Raytrace
    vec3 color = trace(fwd, Normal);
    vec3 oldColor = vec3(0.0);

    imageStore(outTexture, px, vec4(color, 1.0));
}
