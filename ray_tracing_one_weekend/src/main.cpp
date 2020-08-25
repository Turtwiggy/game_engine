
#include "engine/3d/camera.hpp"
#include "engine/renderer/colour.hpp"
using namespace fightingengine;

#include <glm/glm.hpp>

#include <stdio.h>
#include <iostream>
#include <ostream>
#include <fstream>

#define EPSILON 0.0001
#define LARGE_FLOAT 1E+10

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
struct sphere {
    glm::vec3   position;
    float       radius;
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

    int tri_index;
};

glm::vec3 ray_at(const ray r, float t)
{
    return r.origin + (t * r.direction);
}

float length_squared(const glm::vec3& i)
{
    return i.x * i.x + i.y * i.y + i.z * i.z;
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

    info.tri_index = t_index;
    return intersect;
}

bool hit_sphere(const ray& r, const sphere& s, hitinfo& i, double t_min = 0, double t_max = LARGE_FLOAT) {
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

glm::vec3 ray_colour(const ray &r, std::vector<sphere> spheres)
{
    // if (hit_sphere(glm::vec3(0,0,-1), 0.5, r))
    //     return glm::vec3(1, 0, 0);

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

    //hit any sphere?
    float t_nearest = LARGE_FLOAT;
    bool t_hit = false;
    sphere nearest_sphere;
    hitinfo hinfo;

    for(const auto& sphere : spheres)
    {   
        hitinfo hinfo_temp;
        if(hit_sphere(r, sphere, hinfo_temp) && hinfo_temp.t < t_nearest)
        {
            //a closer sphere intersected the ray!
            nearest_sphere = sphere;
            t_nearest = hinfo_temp.t;
            t_hit = true;
            hinfo = hinfo_temp;
        }
    }
    //If we hit a sphere!
    if(t_hit)
    {
        glm::vec3 colour = 0.5f * (hinfo.normal + glm::vec3(1.0, 1.0, 1.0));
        return colour;
    }

    glm::vec3 unit_direction = normalize(r.direction);
    float t = 0.5f*(unit_direction.y + 1.0f);
    return (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
}

void write_color(std::ofstream &out, glm::vec3 pixel_color)
{
    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(255.999 * pixel_color.x) << ' '
        << static_cast<int>(255.999 * pixel_color.y) << ' '
        << static_cast<int>(255.999 * pixel_color.z) << '\n';
}

int main()
{
    printf("beginning... \n");

    Camera c(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    std::ofstream myfile;
    const char *filename = "myimage.ppm";
    myfile.open(filename);

    const float aspect_ratio = 16.0 / 9.0f;
    int width = 200;
    int height = static_cast<int>(width / aspect_ratio);

    myfile << "P3\n" << width << " " << height << "\n255\n";

    //World
    std::vector<sphere> spheres;
    sphere s1;
    s1.position = glm::vec3(0.0, 0.0, -1.0);
    s1.radius = 0.5f;
    sphere s2;
    s2.position = glm::vec3(0.0, -100.5, -1.0);
    s2.radius = 100.0f;
    spheres.push_back(s1);
    spheres.push_back(s2);

    //Camera
    auto viewport_height = 2;
    auto viewport_width = viewport_height * aspect_ratio;
    auto focal_length = 1.0;

    auto origin = glm::vec3(0, 0, 0);
    auto horizontal = glm::vec3(viewport_width, 0, 0);
    auto vertical = glm::vec3(0, viewport_height, 0);
    auto lower_left_corner = origin - (horizontal / 2.0f) - (vertical / 2.0f) - glm::vec3(0, 0, focal_length);

    //Render

    for (int y = height - 1; y >= 0; y--)
    {
        std::cerr << "\rScanlines remaining: " << y << ' ' << std::flush;
        for (int x = 0; x < width; x++)
        {
            float u = float(x) / float(width - 1);
            float v = float(y) / float(height - 1);

            ray r;
            r.origin = origin;
            r.direction = lower_left_corner + u * horizontal + v * vertical - origin;

            glm::vec3 colour = ray_colour(r, spheres);

            write_color(myfile, colour);
        }
    }

    myfile.close();
    std::cerr << "\nDone.\n";
    return 0;
}
