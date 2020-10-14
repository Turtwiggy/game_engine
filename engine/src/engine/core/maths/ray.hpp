#pragma once

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/geometry/triangle.hpp"
#include "engine/geometry/vertex.hpp"
#include "engine/core/maths/base.hpp"

namespace fightingengine
{
    //materials
    #define MATERIAL_METAL 1
    #define MATERIAL_DIFFUSE 2

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
        int material_type; //material_metal or material_diffuse
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

    glm::vec3 ray_at(const Ray r, float t);

    glm::vec3 reflect(const glm::vec3 v, const glm::vec3 n);

    //Ray-Triangle intersections
    bool intersects_triangle(Ray r, FETriangle tri, HitInfo &i);
    bool intersects_any_triangle(Ray r, HitInfo &info, std::vector<FETriangle> &triangles, int set_triangles); 
    
    //Ray-Sphere intersections
    bool hit_sphere(const Ray &r, const Sphere &s, HitInfo &i, double t_min = EPSILON, double t_max = LARGE_FLOAT);
    bool intersects_any_sphere(const Ray &r, const std::vector<Sphere> &spheres, HitInfo &i);

    //Ray-Material
    bool scatter_diffuse(const Ray &r, const HitInfo &h, const Material &m, glm::vec3 &attenuation, Ray &scattered, RandomState &rnd);
    bool scatter_metal(const Ray &r, const HitInfo &h, const Material &m, glm::vec3 &attentuation, Ray &scattered, RandomState &rnd);

} // namespace fightingengine
