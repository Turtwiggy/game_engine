#pragma once

#include "engine/geometry/triangle.hpp"
#include "engine/geometry/vertex.hpp"
#include "engine/core/maths.hpp"

#include <glm/glm.hpp>

namespace fightingengine
{

#define EPSILON 0.001f
#define LARGE_FLOAT 1E+10

    struct Ray
    {
        glm::vec3 origin, direction;
    };
    struct HitInfo
    {
        glm::vec3 point;
        glm::vec3 normal;

        float u;
        float v;
        float t;

        int index;
    };
    struct Material
    {
        int material_type;
        //common to all materials
        glm::vec3 albedo_colour = glm::vec3(0.9f, 0.2f, 0.2f);

        //metal variables
        float metal_fuzz;
    };
    struct Sphere
    {
        glm::vec3 position;
        float radius;
        Material mat;
    };

//materials
#define MATERIAL_METAL 1
#define MATERIAL_DIFFUSE 2

    glm::vec3 ray_at(const Ray r, float t)
    {
        return r.origin + (t * r.direction);
    }

    glm::vec3 reflect(const glm::vec3 v, const glm::vec3 n)
    {
        return v - 2 * glm::dot(v, n) * n;
    }

    //Ray-Triangle intersections

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

    //Ray-Sphere intersections

    bool hit_sphere(const Ray &r, const Sphere &s, HitInfo &i, double t_min = EPSILON, double t_max = LARGE_FLOAT)
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

    //Ray-Material

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

} // namespace fightingengine
