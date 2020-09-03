#version 430 core

layout(binding = 0, rgba16f) readonly uniform image2D normalTexture;
layout(binding = 1, rgba16f) writeonly uniform image2D outTexture;

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

// ---- UNIFORMS ----

uniform int set_triangles;
uniform float time, viewport_width, viewport_height;
uniform vec3 eye, lower_left_corner;

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
vec4 randomHemispherePoint(vec3 n, vec2 rand);
float hemisphereProbability(vec3 n, vec3 v);
vec4 randomDiskPoint(vec3 n, float d, float r, vec2 rand);
float diskProbability(vec3 n, float d, float r, vec3 v);

//GLOBAL
ivec2 px;

#define SPHERES_IN_SCENE 4
sphere[SPHERES_IN_SCENE] spheres;

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

    for (int index = 0; index < SPHERES_IN_SCENE; index++)
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
    //scattered.direction = reflected;

    attentuation = m.albedo_colour;

    return (dot(scattered.direction, h.normal) > 0);
}

vec3 trace(ray r, vec3 normal) 
{
    return vec3(0.3, 0.6, 0.3);

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
        return;

    vec2 p = (vec2(px) + vec2(0.5)) / vec2(size);

    ray fwd;
    fwd.origin = eye;
    fwd.direction = lower_left_corner + px.x*viewport_width + px.y*viewport_height - eye;

    //Sample textures
    //vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalTexture, px).rgb;

    //Raytrace
    vec3 color = trace(fwd, Normal);
    vec3 oldColor = vec3(0.0);

    imageStore(outTexture, px, vec4(color, 1.0));
}