#version 430 core

layout(binding = 0, rgba16f) writeonly uniform image2D outTexture;
layout(binding = 1, rgba16f) readonly uniform image2D normalTexture;
layout( std430, binding = 2 ) readonly buffer bufferData
{
    triangle triangles[];
};

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI (1.0 / PI)
#define ONE_OVER_2PI (1.0 / TWO_PI)

#define LARGE_FLOAT 1E+10
#define EPSILON 0.0001

#define MATERIAL_METAL 1
#define MATERIAL_DIFFUSE 2

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
}
struct material {
    int material_type;
    vec3 albedo_colour;

    //metal variables
    float metal_fuzz;
}
struct sphere 
{
    vec3 position;
    float radius;
    material mat;
}

vec3 ray_at(const ray r, float t) 
{
    return r.origin + (t * r.direction);
}

vec3 reflect(const vec3 v, const vec3 n)
{
    return v - 2 * dot(v, n) * n;
}

uniform int set_triangles;
uniform float time;
uniform vec3 eye, ray00, ray01, ray10, ray11;

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


    bool intersects_triangle(Ray r, FETriangle tri, HitInfo &i)
    {
        glm::vec3 v0 = tri.p0.Position;
        glm::vec3 v1 = tri.p1.Position;
        glm::vec3 v2 = tri.p2.Position;
        glm::vec3 dir = r.direction;
        glm::vec3 orig = r.origin;

        glm::vec3 v0v1 = v1 - v0;
        glm::vec3 v0v2 = v2 - v0;

        glm::vec3 pvec = cross(dir, v0v2);
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

        glm::vec3 tvec = orig - v0;
        float u = dot(tvec, pvec) * inv_det;
        if (u < 0.0 || u > 1.0)
        {
            return false;
        }

        glm::vec3 qvec = cross(tvec, v0v1);
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

    bool intersects_any_triangle(Ray r, HitInfo &info, std::vector<FETriangle> &triangles, int set_triangles)
    {
        float t_nearest = LARGE_FLOAT;
        bool intersect = false;
        int t_index;

        for (int i = 0; i < set_triangles; i++)
        {
            HitInfo h;
            const FETriangle tri = triangles[i];
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

    bool hit_sphere(const Ray &r, const Sphere &s, HitInfo &i, double t_min, double t_max)
        {
            //sphere intersection code
            glm::vec3 oc = r.origin - s.position;
            float a = length_squared(r.direction);
            float half_b = dot(oc, r.direction);
            float c = length_squared(oc) - s.radius * s.radius;
            float discriminant = half_b * half_b - a * c;

            if (discriminant > 0)
            {
                auto root = sqrt(discriminant);
                auto temp = (-half_b - root) / a;
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

    bool intersects_any_sphere(const Ray &r, const std::vector<Sphere> &spheres, HitInfo &i)
    {
        float t_nearest = LARGE_FLOAT;
        bool t_hit = false;
        HitInfo hinfo;

        for (int index = 0; index < spheres.size(); index++)
        {
            HitInfo hinfo_temp;

            if (hit_sphere(r, spheres[index], hinfo_temp) && hinfo_temp.t < t_nearest)
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

    bool scatter_diffuse(const Ray &r, const HitInfo &h, const Material &m, glm::vec3 &attenuation, Ray &scattered, RandomState &rnd)
    {
        // if(m.material_type != MATERIAL_DIFFUSE)
        //     return false;

        glm::vec3 target = h.normal + rand_unit_vector(rnd);

        scattered.origin = h.point;
        scattered.direction = target;

        attenuation = m.albedo_colour;

        return true;
    }

    bool scatter_metal(const Ray &r, const HitInfo &h, const Material &m, glm::vec3 &attentuation, Ray &scattered, RandomState &rnd)
    {
        // if(m.material_type != MATERIAL_METAL)
        //     return false;

        glm::vec3 reflected = reflect(normalize(r.direction), h.normal);

        scattered.origin = h.point;
        scattered.direction = reflected + m.metal_fuzz * rand_unit_vector(rnd);
        //scattered.direction = reflected;

        attentuation = m.albedo_colour;

        return (dot(scattered.direction, h.normal) > 0);
    }


vec3 trace(ray r, vec3 normal) {

    //vec3 att = vec3(1.0);
    bool intersected = false;

    vec3 colour = vec3(1.0);
    ray next_ray = r;

    for(int bounce = 2; bounce > -1; bounce-- ) {

        if(bounce <= 0)
        {
            return vec3(0.0);
        }

        hitinfo i;

        if(intersects_any_triangle(next_ray, i))
        {
            triangle tri = triangles[i.tri_index];

            vec3 albedo_colour = tri.p0.colour.xyz;

            if(bounce == 2)
            {
                colour = albedo_colour;
            } else
            {
                colour *= 0.5;
            }

            vec3 rand = randvec3(bounce);
            vec3 target = i.point + randomHemispherePoint(i.normal, rand.xy).xyz;

            //att *= brdf(albedo_colour, s.xyz, -next_ray.direction, i.normal);

            ray new_ray;
            //new_ray.origin = i.point + i.normal * EPSILON;
            new_ray.origin = i.point;
            new_ray.direction = target - i.point;
            next_ray = new_ray;

            //att *= max(0.0, dot( new_ray.direction, i.normal));
            //if(s.w > 0.0)
            //    att /= s.w;

            intersected = true;
        } else
        {
            colour *= no_hit_color(next_ray);
            return colour;
        }

        //return 0.5 * vec3( new_ray.direction.x + 1,  new_ray.direction.y + 1,  new_ray.direction.z + 1);
    }

    return vec3(1.0, 0.0, 1.0);

    //return the normal colour
    //return 0.5 * vec3(intersection_point_normal.x + 1, intersection_point_normal.y + 1, intersection_point_normal.z + 1);
    //return the triangles's v0 colour!
    //return tri.p0.colour.xyz;
}

layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {

    ivec2 px = ivec2(gl_GlobalInvocationID.xy);

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