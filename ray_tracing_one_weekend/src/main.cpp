#include "engine/3d/camera.hpp"
#include "engine/3d/ray.hpp"
#include "engine/renderer/colour.hpp"
#include "engine/core/random.hpp"
#include "engine/core/maths.hpp"
using namespace fightingengine;

#include <glm/glm.hpp>
using namespace glm;

#include <stdio.h>
#include <iostream>
#include <ostream>
#include <fstream>

#define SAMPLES_PER_PIXEL 50
#define BOUNCES 35

vec3 ray_colour(const Ray &r, std::vector<Sphere> spheres, RandomState& rnd)
{
    // FETriangle tri;
    // Vertex v0;
    // v0.Position = vec3(0.0, 0.0, -1.0);
    // Vertex v1;
    // v1.Position = vec3(400.0, 400.0, -1.0);
    // Vertex v2;
    // v2.Position = vec3(400.0, 0.0, -1.0);
    // tri.p0 = v0;
    // tri.p1 = v1;
    // tri.p2 = v2;
    // hitinfo i;
    // if (intersects_triangle(r, tri, i))
    //     return vec3(1.0, 0.0, 0.0);

    HitInfo i;
    Ray ray_to_shoot = r;
    vec3 final_attenuation = vec3(1.0, 1.0, 1.0);
    bool intersected = false;

    for(int index = 0; index < BOUNCES; index++)
    {
        if(intersects_any_sphere(ray_to_shoot, spheres, i))
        {
            intersected = true;
            const Sphere& hit_sphere = spheres[i.index];

            Ray scattered_ray;
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

void write_color(std::ofstream &out, vec3 pixel_color)
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
    out << static_cast<int>(256 * clamp(r, 0.0f, 0.999f)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0f, 0.999f)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0f, 0.999f)) << '\n';
}

int main()
{
    printf("beginning... \n");

    std::ofstream myfile;
    const char *filename = "myimage.ppm";
    myfile.open(filename);

    const float aspect_ratio = 16.0 / 9.0f;
    int width = 250;
    int height = static_cast<int>(width / aspect_ratio);

    myfile << "P3\n" << width << " " << height << "\n255\n";

    //ground sphere
    Sphere s4;
    s4.position = vec3(0.0, -100.5, -1.0);
    s4.radius = 100.0f;
    s4.mat.material_type = MATERIAL_DIFFUSE;
    s4.mat.albedo_colour = vec3(0.8, 0.8, 0.0);
    //main sphere
    Sphere s1;
    s1.position = vec3(0.0, 0.0, -1.0);
    s1.radius = 0.5f;
    s1.mat.material_type = MATERIAL_DIFFUSE;
    s1.mat.albedo_colour = vec3(0.7, 0.3, 0.3);
    //left sphere
    Sphere s2;
    s2.position = vec3(-1.0, 0.0, -1.0);
    s2.radius = 0.5f;
    s2.mat.material_type = MATERIAL_METAL;
    s2.mat.albedo_colour = vec3(0.8, 0.8, 0.8);
    s2.mat.metal_fuzz = 0.3f;
    //right sphere
    Sphere s3;
    s3.position = vec3(1.0, 0.0, -1.0);
    s3.radius = 0.5f;
    s3.mat.material_type = MATERIAL_METAL;
    s3.mat.albedo_colour = vec3(0.8, 0.6, 0.2);
    s3.mat.metal_fuzz = 0.8f;
    //World
    std::vector<Sphere> spheres;
    spheres.push_back(s1);
    spheres.push_back(s2);
    spheres.push_back(s3);
    spheres.push_back(s4);

    //Camera
    auto viewport_height = 2;
    auto viewport_width = viewport_height * aspect_ratio;

    Camera cam(
        vec3(0.0f, 0.0f, 0.0f), 
        vec3(0.0f, 1.0f, 0.0f),
        viewport_width, 
        viewport_height );

    RandomState rnd;

    //Render

    for (int y = height - 1; y >= 0; y--)
    {
        std::cerr << "\rScanlines remaining: " << y << ' ' << std::flush;
        for (int x = 0; x < width; x++)
        {
            vec3 colour(0, 0, 0);
            
            //multiple pixel sampling (anti-aliasing)
            for (int s = 0; s < SAMPLES_PER_PIXEL; ++s) 
            {
                float rng_val = rand_det_s(rnd.rng, 0.0, 1.0);
                //printf("rng: %f", rng_val);
                float u = float(x + rng_val) / float(width - 1);
                float v = float(y + rng_val) / float(height - 1);

                Ray& r = cam.get_ray(u, v, viewport_width, viewport_height);

                colour += ray_colour(r, spheres, rnd);
            }

            write_color(myfile, colour);
        }
    }

    myfile.close();
    std::cerr << "\nDone.\n";
    return 0;
}
