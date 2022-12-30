#version 430 core

// ---- STRUCTS ----

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
struct ray {
    vec3 origin, direction;
};
struct hit_info {
    vec3 point;
    vec3 normal;
    float u;
    float v;
    float t;
    int index;
};
struct material {
    int material_type;
    vec3 albedo_colour;

    //metal variables
    float metal_fuzz;
};
struct sphere {
    vec3 position;
    float radius;
    material mat;
};

// ---- COMPUTE SHADRER LAYOUT ----

layout(binding = 0, rgba16f) readonly uniform image2D normalTexture;
layout(binding = 1, rgba16f) writeonly uniform image2D outTexture;
layout( std430, binding = 2 ) readonly buffer bufferData
{
    triangle triangles[];
};

// ---- DEFINES ----

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI (1.0 / PI)
#define ONE_OVER_2PI (1.0 / TWO_PI)
#define LARGE_FLOAT 1E+10
#define EPSILON 0.0001

#define MATERIAL_METAL 1
#define MATERIAL_DIFFUSE 2

#define SAMPLES_PER_PIXEL 1
#define BOUNCES 50

// ---- UNIFORMS ----

uniform int set_triangles;
uniform float time;
uniform vec3 eye, ray00, ray01, ray10, ray11;
vec3 light_source;

// ---- GLOBALS ---- 
ivec2 px;

#define SPHERE_LENGTH 5
sphere[SPHERE_LENGTH] spheres;

// ---- FUNCTIONS ----

vec3 ray_at(const ray r, float t) 
{
    return r.origin + (t * r.direction);
}
vec3 reflect(const vec3 v, const vec3 n)
{
    return v - 2 * dot(v, n) * n;
}

// See random.glsl for more explanation of these functions.
float random(vec3 f);
vec4 randomCosineWeightedHemispherePoint(vec3 n, vec2 rand);
vec4 randomPhongWeightedHemispherePoint(vec3 r, float a, vec2 rand);
vec4 randomDiskPoint(vec3 n, float d, float r, vec2 rand);
float diskProbability(vec3 n, float d, float r, vec3 v);

vec3 randvec3() {
  return vec3(
    random(vec3(px, time)),
    random(vec3(px, time * 1.1)),
    random(vec3(px, time * 0.3)));
}

float length_squared(const vec3 i)
{
    return i.x * i.x + i.y * i.y + i.z * i.z;
}

float scale(float value, float min, float max)
{
    return ((max - min)*(value-min))/(max-min)+min;
}

vec3 rand_unit_vector()
{
    float a = scale( random(vec3(px, time)), 0, TWO_PI);
    float z = scale( random(vec3(px, time)), -1, 1);
    float r = sqrt(1.0 - z * z);
    return vec3(r * cos(a), r * sin(a), z);
}

// ---- RAY TRIANGLE

bool intersects_triangle(ray r, triangle tri, inout hit_info i)
{
    vec3 v0 = tri.p0.pos.xyz;
    vec3 v1 = tri.p1.pos.xyz;
    vec3 v2 = tri.p2.pos.xyz;
    vec3 dir = r.direction;
    vec3 orig = r.origin;

    vec3 v0v1 = v1 - v0;
    vec3 v0v2 = v2 - v0;

    vec3 pvec = cross(dir, v0v2);
    float det = dot(v0v1, pvec);

    //culling
    // if(det < EPSILON) {
    //    return false;
    // }

    //no culling
    if (abs(det) < EPSILON)
    {
        return false;
    }

    float inv_det = 1 / det;

    vec3 tvec = orig - v0;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0)
    {
        return false;
    }

    vec3 qvec = cross(tvec, v0v1);
    float v = dot(dir, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0)
    {
        return false;
    }

    float intersection_distance = dot(v0v2, qvec) * inv_det;
    if (intersection_distance > EPSILON)
    {
        //intersection point
        i.point = (orig + dir * intersection_distance);

        //intersection normal
        i.normal = normalize(cross(v0v1, v0v2));

        i.t = intersection_distance;

        i.u = u;
        i.v = v;

        return true;
    }
    return false; // this ray hits the triangle
}

bool intersects_any_triangle(ray r, inout hit_info info, int set_triangles)
{
    float t_nearest = LARGE_FLOAT;
    bool intersect = false;
    int t_index;

    for (int i = 0; i < set_triangles; i++)
    {
        hit_info h;
        const triangle tri = triangles[i];
        if (intersects_triangle(r, tri, h) && h.t < t_nearest)
        {
            //a closer triangle intersected the ray!
            intersect = true;

            t_nearest = h.t;
            t_index = i;
        }
    }

    info.index = t_index;
    return intersect;
}

// ---- RAY SPHERE

bool hit_sphere(const ray r, const sphere s, inout hit_info i, double t_min, double t_max)
{
    //sphere intersection code
    vec3 oc = r.origin - s.position;
    float a = length_squared(r.direction);
    float half_b = dot(oc, r.direction);
    float c = length_squared(oc) - s.radius * s.radius;
    float discriminant = half_b * half_b - a * c;

    if (discriminant > 0)
    {
        float root = sqrt(discriminant);
        float temp = (-half_b - root) / a;
        if (temp < t_max && temp > t_min)
        {
            i.t = temp;
            i.point = ray_at(r, i.t);
            i.normal = (i.point - s.position) / s.radius;

            //is a outward-facing face?
            bool outwards = dot(r.direction, i.normal) < 0;

            if(!outwards)
                return false;

            return true;
        }
        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min)
        {
            i.t = temp;
            i.point = ray_at(r, i.t);
            i.normal = (i.point - s.position) / s.radius;

            //is a outward-facing face?
            bool outwards = dot(r.direction, i.normal) < 0;
            
            if(!outwards)
                return false;

            return true;
        }
    }
    return false;
}

bool intersects_any_sphere(const ray r, inout hit_info i)
{
    float t_nearest = LARGE_FLOAT;
    bool t_hit = false;
    hit_info hinfo;

    for (int index = 0; index < SPHERE_LENGTH; index++)
    {
        hit_info hinfo_temp;

        if (hit_sphere(r, spheres[index], hinfo_temp, EPSILON, LARGE_FLOAT) && hinfo_temp.t < t_nearest)
        {
            //a closer sphere intersected the ray!
            t_nearest = hinfo_temp.t;
            t_hit = true;

            hinfo = hinfo_temp;
            hinfo.index = index;
        }
    }

    if (t_hit)
    {
        i = hinfo;
        return true;
    }
    return false;
}

// ---- RAY COLOUR

bool scatter_diffuse(const ray r, const hit_info h, const material m, inout vec3 attenuation, inout ray scattered)
{
    // if(m.material_type != MATERIAL_DIFFUSE)
    //     return false;

    vec3 target = h.normal + rand_unit_vector();

    scattered.origin = h.point;
    scattered.direction = target;

    attenuation = m.albedo_colour;

    return true;
}

bool scatter_metal(const ray r, const hit_info h, const material m, inout vec3 attentuation, inout ray scattered)
{
    // if(m.material_type != MATERIAL_METAL)
    //     return false;

    vec3 reflected = reflect(normalize(r.direction), h.normal);

    scattered.origin = h.point;
    scattered.direction = reflected + m.metal_fuzz * rand_unit_vector();

    attentuation = m.albedo_colour;

    return (dot(scattered.direction, h.normal) > 0);
}

// ---- RayTrace function

vec3 trace(ray r, vec3 normal) 
{
    hit_info i;
    ray ray_to_shoot = r;
    vec3 final_attenuation = vec3(1.0, 1.0, 1.0);
    bool intersected = false;

    for(int index = 0; index < BOUNCES; index++ )
    {
        if(intersects_any_sphere(ray_to_shoot, i))
        {
            intersected = true;
            const sphere hit_sphere = spheres[i.index];

            ray scattered_ray;
            bool scatter = false;
            vec3 attenuation;

            //We hit another object on the final bounce... but
            //we're at the bounce limit... 
            if(index == BOUNCES - 1){
                final_attenuation = vec3(0.0, 0.0, 0.0);
                break;
            }

            switch(hit_sphere.mat.material_type)
            {
                case MATERIAL_DIFFUSE:
                        scatter = scatter_diffuse(ray_to_shoot, i, hit_sphere.mat, attenuation, scattered_ray);
                        final_attenuation *= attenuation;
                    break;
                case MATERIAL_METAL:
                        scatter = scatter_metal(ray_to_shoot, i, hit_sphere.mat, attenuation, scattered_ray);
                        final_attenuation *= attenuation;
                    break;
            }

            if(!scatter)
            {
                attenuation = vec3(0.0, 0.0, 0.0);
                break;
            } 

            ray_to_shoot = scattered_ray;
            continue;
        }
        break;
    }

    vec3 unit_direction = normalize(ray_to_shoot.direction);
    float t = 0.5f*(unit_direction.y + 1.0f);
    vec3 sky_colour = (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);

    return final_attenuation*sky_colour;
}

layout (local_size_x = 16, local_size_y = 8) in;

void main(void) {

    px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outTexture);

    if (any(greaterThanEqual(px, size)))
        return;

    //Sample textures
    //vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalTexture, px).rgb;

    // ---- Setup world
    light_source = vec3(10, 10, 10);

    //ground sphere
    sphere s4;
    s4.position = vec3(0.0, -100.5, -1.0);
    s4.radius = 100.0f;
    s4.mat.material_type = MATERIAL_DIFFUSE;
    s4.mat.albedo_colour = vec3(0.8, 0.8, 0.0);
    //main sphere
    sphere s1;
    s1.position = vec3(0.0, 0.0, -1.0);
    s1.radius = 0.5f;
    s1.mat.material_type = MATERIAL_DIFFUSE;
    s1.mat.albedo_colour = vec3(0.7, 0.3, 0.3);
    //left sphere
    sphere s2;
    s2.position = light_source;
    s2.radius = 0.5f;
    s2.mat.material_type = MATERIAL_METAL;
    s2.mat.albedo_colour = vec3(0.8, 0.8, 0.8);
    s2.mat.metal_fuzz = 0.0f;
    //right sphere
    sphere s3;
    s3.position = vec3(2.0, 0.0, 0.0);
    s3.radius = 0.5f;
    s3.mat.material_type = MATERIAL_METAL;
    s3.mat.albedo_colour = vec3(0.8, 0.6, 0.2);
    s3.mat.metal_fuzz = 0.0f;

    //player sphere
    sphere s5;
    s5.position = eye;
    s5.radius = 0.2f;
    s5.mat.material_type = MATERIAL_DIFFUSE;
    s5.mat.albedo_colour = vec3(0.1, 0.0, 0.0);
    s5.mat.metal_fuzz = 0.0f;

    spheres[0] = s1;
    spheres[1] = s2;
    spheres[2] = s3;
    spheres[3] = s4;
    spheres[4] = s5;

    //Raytrace
    vec3 colour = vec3(0, 0, 0);

    for(int s = 0; s < SAMPLES_PER_PIXEL; ++s)
    {
        float rng_val = random(vec3(px, time));
        vec2 px_rnd = vec2(px.x + rng_val, px.y + rng_val);

        vec2 p = (vec2(px_rnd) + vec2(0.5)) / vec2(size);
        vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);
        ray fwd = {eye, dir};

        colour += trace(fwd, Normal);
    }

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    float scale = 1.0 / SAMPLES_PER_PIXEL;
    colour.x = clamp(sqrt(scale*colour.x), 0.0, 1.0);
    colour.y = clamp(sqrt(scale*colour.y), 0.0, 1.0);
    colour.z = clamp(sqrt(scale*colour.z), 0.0, 1.0);

    imageStore(outTexture, px, vec4(colour, 1.0));
}



// /**
//  * Evaluate the specular part of the BRDF.
//  *
//  * @param b the box to evaluate (used to get its diffuse color)
//  * @param i the incoming light direction
//  *          (by convention this points away from the surface)
//  * @param o the outgoing light direction
//  * @param n the surface normal
//  * @returns the attenuation factor
//  */
// vec3 brdfSpecular(box b, vec3 i, vec3 o, vec3 n) {
//   float a = phongExponent;
//   vec3 r = reflect(-i, n);
//   return vec3(pow(max(0.0, dot(r, o)), a) * (a + 2.0) * ONE_OVER_2PI);
// }

// /**
//  * Evaluate the diffuse part of the BRDF.
//  *
//  * @param albedo the diffuse color
//  * @param i the incoming light direction
//  *          (by convention this points away from the surface)
//  * @param o the outgoing light direction
//  * @param n the surface normal
//  * @returns the attenuation factor
//  */
// vec3 brdfDiffuse(box b, vec3 i, vec3 o, vec3 n) {
//   return b.col * ONE_OVER_PI;
// }

// /**
//  * Compute the BRDF of the box's surface given the incoming and outgoing
//  * light directions as well as the surface normal.
//  *
//  * @param b the box to evaluate (used to get its diffuse color)
//  * @param i the incoming light direction
//  *          (by convention this points away from the surface)
//  * @param o the outgoing light direction
//  * @param n the surface normal
//  * @returns the attenuation factor
//  */
// vec3 brdf(box b, vec3 i, vec3 o, vec3 n) {
//   return brdfSpecular(b, i, o, n) * specularFactor
//          +
//          brdfDiffuse(b, i, o, n) * (1.0 - specularFactor);
// }