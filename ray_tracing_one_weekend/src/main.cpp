
#include "engine/3d/camera.hpp"
#include "engine/renderer/colour.hpp"
#include "engine/core/random.hpp"
#include "engine/core/maths.hpp"
using namespace fightingengine;

#include <glm/glm.hpp>

#include <stdio.h>
#include <iostream>
#include <ostream>
#include <fstream>

#define EPSILON 0.001f
#define LARGE_FLOAT 1E+10
#define SAMPLES_PER_PIXEL 70
#define BOUNCES 35

//materials 
#define MATERIAL_METAL 1
#define MATERIAL_DIFFUSE 2

struct vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex;
    glm::vec3 colour;
};
struct triangle
{
    vertex p0;
    vertex p1;
    vertex p2;
};
struct material {
    int material_type;
    //common to all materials
    glm::vec3 albedo_colour = glm::vec3(0.9f, 0.2f, 0.2f);

    //metal variables
    float metal_fuzz;
};
struct sphere {
    glm::vec3   position;
    float       radius;
    material    mat;    
};
struct ray
{
    glm::vec3 origin, direction;
};
struct hitinfo
{
    glm::vec3 point;
    glm::vec3 normal;

    float u;
    float v;
    float t;

    int index;
};


glm::vec3 ray_at(const ray r, float t)
{
    return r.origin + (t * r.direction);
}

glm::vec3 reflect(const glm::vec3 v, const glm::vec3 n)
{
    return v - 2*glm::dot(v, n)*n;
}


bool scatter_diffuse(const ray& r, const hitinfo& h, const material& m, glm::vec3& attenuation, ray& scattered, random_state& rnd)
{
    if(m.material_type != MATERIAL_DIFFUSE)
        return false;

    glm::vec3 target = h.normal + rand_unit_vector(rnd);

    scattered.origin = h.point;
    scattered.direction = target;

    attenuation = m.albedo_colour;

    return true;
}

bool scatter_metal(const ray& r, const hitinfo& h, const material& m, glm::vec3& attentuation, ray& scattered, random_state& rnd)
{
    if(m.material_type != MATERIAL_METAL)
        return false;

    glm::vec3 reflected = reflect(normalize(r.direction), h.normal);

    scattered.origin = h.point;
    // scattered.direction = reflected + m.metal_fuzz*rand_unit_vector(rnd);
    scattered.direction = reflected;

    attentuation = m.albedo_colour;

    return (dot(scattered.direction, h.normal) > 0.0);
}


bool intersects_triangle(ray r, triangle tri, hitinfo &i)
{
    glm::vec3 v0 = tri.p0.pos;
    glm::vec3 v1 = tri.p1.pos;
    glm::vec3 v2 = tri.p2.pos;
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

bool intersects_any_triangle(ray r, hitinfo &info, std::vector<triangle> &triangles, int set_triangles)
{
    float t_nearest = LARGE_FLOAT;
    bool intersect = false;
    int t_index;

    for (int i = 0; i < set_triangles; i++)
    {
        hitinfo h;
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


bool hit_sphere(const ray& r, const sphere& s, hitinfo& i, double t_min = EPSILON, double t_max = LARGE_FLOAT) {
    //sphere intersection code
    glm::vec3 oc = r.origin - s.position;
    float a = length_squared(r.direction);
    float half_b = dot(oc, r.direction);
    float c = length_squared(oc) - s.radius*s.radius;
    float discriminant = half_b * half_b - a*c;

    if (discriminant > 0) {
        auto root = sqrt(discriminant);
        auto temp = (-half_b - root) / a;
        if (temp < t_max && temp > t_min) {
            i.t = temp;
            i.point = ray_at(r, i.t);
            i.normal = (i.point - s.position) / s.radius;

            //is a outward-facing face?
            bool outwards = dot(r.direction, i.normal) < 0;

            return true;
        }
        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min) {
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

bool intersects_any_sphere(const ray& r, const std::vector<sphere>& spheres, hitinfo& i)
{
    float t_nearest = LARGE_FLOAT;
    bool t_hit = false;
    hitinfo hinfo;

    for(int index = 0; index < spheres.size(); index++)
    {   
        hitinfo hinfo_temp;

        if(hit_sphere(r, spheres[index], hinfo_temp) && hinfo_temp.t < t_nearest)
        {
            //a closer sphere intersected the ray!
            t_nearest = hinfo_temp.t;
            t_hit = true;

            hinfo = hinfo_temp;
            hinfo.index = index;
        }
    }

    if(t_hit)
    {
        i = hinfo;
        return true;
    }
    return false;
}


glm::vec3 ray_colour(const ray &r, std::vector<sphere> spheres, random_state& rnd)
{
    triangle tri;
    vertex v0;
    v0.pos = glm::vec3(0.0, 0.0, -1.0);
    vertex v1;
    v1.pos = glm::vec3(400.0, 400.0, -1.0);
    vertex v2;
    v2.pos = glm::vec3(400.0, 0.0, -1.0);
    tri.p0 = v0;
    tri.p1 = v1;
    tri.p2 = v2;
    // hitinfo i;
    // if (intersects_triangle(r, tri, i))
    //     return glm::vec3(1.0, 0.0, 0.0);

    hitinfo i;
    ray ray_to_shoot = r;
    glm::vec3 final_attenuation = glm::vec3(1.0, 1.0, 1.0);
    bool intersected = false;

    for(int index = 0; index < BOUNCES; index++)
    {
        if(intersects_any_sphere(ray_to_shoot, spheres, i))
        {
            intersected = true;
            const sphere& hit_sphere = spheres[i.index];

            ray scattered_ray;
            bool scatter = false;
            glm::vec3 attenuation;

            //We hit another object on the final bounce... but
            //we're at the bounce limit... 
            if(index == BOUNCES - 1){
                final_attenuation *= glm::vec3(0.0, 0.0, 0.0);
                break;
            }

            switch(hit_sphere.mat.material_type)
            {
                case MATERIAL_DIFFUSE:
                        scatter = scatter_diffuse(ray_to_shoot, i, hit_sphere.mat, attenuation, scattered_ray, rnd);
                        final_attenuation *= attenuation;
                    break;
                case MATERIAL_METAL:
                        scatter = scatter_metal(ray_to_shoot, i, hit_sphere.mat, attenuation, scattered_ray, rnd);
                        final_attenuation *= attenuation;
                    break;
            }

            if(!scatter)
            {
                attenuation = glm::vec3(0.0, 0.0, 0.0);
                break;
            } 

            ray_to_shoot = scattered_ray;
            continue;
        }
        break;
    }

    glm::vec3 unit_direction = normalize(ray_to_shoot.direction);
    float t = 0.5f*(unit_direction.y + 1.0f);
    glm::vec3 sky_colour = (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);

    return final_attenuation*sky_colour;
}

void write_color(std::ofstream &out, glm::vec3 pixel_color)
{
    auto r = pixel_color.x;
    auto g = pixel_color.y;
    auto b = pixel_color.z;

    // Divide the color by the number of samples and gamma-correct for gamma=2.0.
    auto scale = 1.0 / SAMPLES_PER_PIXEL;
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(256 * glm::clamp(r, 0.0f, 0.999f)) << ' '
        << static_cast<int>(256 * glm::clamp(g, 0.0f, 0.999f)) << ' '
        << static_cast<int>(256 * glm::clamp(b, 0.0f, 0.999f)) << '\n';
}

int main()
{
    printf("beginning... \n");

    Camera c(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    std::ofstream myfile;
    const char *filename = "myimage.ppm";
    myfile.open(filename);

    const float aspect_ratio = 16.0 / 9.0f;
    int width = 250;
    int height = static_cast<int>(width / aspect_ratio);

    myfile << "P3\n" << width << " " << height << "\n255\n";

    //ground sphere
    sphere s4;
    s4.position = glm::vec3(0.0, -100.5, -1.0);
    s4.radius = 100.0f;
    s4.mat.material_type = MATERIAL_DIFFUSE;
    s4.mat.albedo_colour = glm::vec3(0.8, 0.8, 0.0);
    //main sphere
    sphere s1;
    s1.position = glm::vec3(0.0, 0.0, -1.0);
    s1.radius = 0.5f;
    s1.mat.material_type = MATERIAL_DIFFUSE;
    s1.mat.albedo_colour = glm::vec3(0.7, 0.3, 0.3);
    //left sphere
    sphere s2;
    s2.position = glm::vec3(-1.0, 0.0, -1.0);
    s2.radius = 0.5f;
    s2.mat.material_type = MATERIAL_METAL;
    s2.mat.albedo_colour = glm::vec3(0.8, 0.8, 0.8);
    s2.mat.metal_fuzz = 0.3f;
    //right sphere
    sphere s3;
    s3.position = glm::vec3(1.0, 0.0, -1.0);
    s3.radius = 0.5f;
    s3.mat.material_type = MATERIAL_METAL;
    s3.mat.albedo_colour = glm::vec3(0.8, 0.6, 0.2);
    s3.mat.metal_fuzz = 0.8f;

    //World
    std::vector<sphere> spheres;
    spheres.push_back(s1);
    spheres.push_back(s2);
    spheres.push_back(s3);
    spheres.push_back(s4);

    //Camera
    auto viewport_height = 2;
    auto viewport_width = viewport_height * aspect_ratio;
    auto focal_length = 1.0;

    auto origin = glm::vec3(0, 0, 0);
    auto horizontal = glm::vec3(viewport_width, 0, 0);
    auto vertical = glm::vec3(0, viewport_height, 0);
    auto lower_left_corner = origin - (horizontal / 2.0f) - (vertical / 2.0f) - glm::vec3(0, 0, focal_length);

    random_state rnd;

    //Render

    for (int y = height - 1; y >= 0; y--)
    {
        std::cerr << "\rScanlines remaining: " << y << ' ' << std::flush;
        for (int x = 0; x < width; x++)
        {
            glm::vec3 colour(0, 0, 0);
            
            //multiple pixel sampling (anti-aliasing)
            for (int s = 0; s < SAMPLES_PER_PIXEL; ++s) 
            {
                float rng_val = rand_det_s(rnd.rng, 0.0, 1.0);
                //printf("rng: %f", rng_val);
                float u = float(x + rng_val) / float(width - 1);
                float v = float(y + rng_val) / float(height - 1);

                ray r;
                r.origin = origin;
                r.direction = lower_left_corner + u * horizontal + v * vertical - origin;

                colour += ray_colour(r, spheres, rnd);
            }

            write_color(myfile, colour);
        }
    }

    myfile.close();
    std::cerr << "\nDone.\n";
    return 0;
}
