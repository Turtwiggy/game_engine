
// //header
// #include "engine/core/maths/ray.hpp"

// //other library headers
// #include "glm/glm.hpp"

// //your project headers
// #include "engine/core/maths/base.hpp"

// //using this namespace so that
// //it copies and pastes easier in to
// //the raytracer.glsl shader
// using namespace glm;

// namespace fightingengine
// {

// vec3 ray_at(const Ray r, float t)
// {
//     return r.origin + (t * r.direction);
// }

// vec3 reflect(const vec3 v, const vec3 n)
// {
//     return v - 2 * dot(v, n) * n;
// }

// bool intersects_triangle(Ray r, Triangle tri, HitInfo &i)
// {
//     vec3 v0 = tri.p0.Position;
//     vec3 v1 = tri.p1.Position;
//     vec3 v2 = tri.p2.Position;
//     vec3 dir = r.direction;
//     vec3 orig = r.origin;

//     vec3 v0v1 = v1 - v0;
//     vec3 v0v2 = v2 - v0;

//     vec3 pvec = cross(dir, v0v2);
//     float det = dot(v0v1, pvec);

//     //culling
//     // if(det < EPSILON) {
//     //    return false;
//     // }

//     //no culling
//     if (abs(det) < EPSILON)
//     {
//         return false;
//     }

//     float inv_det = 1 / det;

//     vec3 tvec = orig - v0;
//     float u = dot(tvec, pvec) * inv_det;
//     if (u < 0.0 || u > 1.0)
//     {
//         return false;
//     }

//     vec3 qvec = cross(tvec, v0v1);
//     float v = dot(dir, qvec) * inv_det;
//     if (v < 0.0 || u + v > 1.0)
//     {
//         return false;
//     }

//     float intersection_distance = dot(v0v2, qvec) * inv_det;
//     if (intersection_distance > EPSILON)
//     {
//         //intersection point
//         i.point = (orig + dir * intersection_distance);

//         //intersection normal
//         i.normal = normalize(cross(v0v1, v0v2));

//         i.t = intersection_distance;

//         i.u = u;
//         i.v = v;

//         return true;
//     }
//     return false; // this ray hits the triangle
// }

// bool intersects_any_triangle(Ray r, HitInfo &info, std::vector<Triangle>
// &triangles, int set_triangles)
// {
//     float t_nearest = LARGE_FLOAT;
//     bool intersect = false;
//     int t_index;

//     for (int i = 0; i < set_triangles; i++)
//     {
//         HitInfo h;
//         const Triangle tri = triangles[i];
//         if (intersects_triangle(r, tri, h) && h.t < t_nearest)
//         {
//             //a closer triangle intersected the ray!
//             intersect = true;

//             t_nearest = h.t;
//             t_index = i;
//         }
//     }

//     info.index = t_index;
//     return intersect;
// }

// vec2 get_sphere_uv(const vec3& p) {
//     float phi = atan(p.z, p.x);
//     float theta = asin(p.y);
//     float u = 1-(phi + PI) / (2*PI);
//     float v = (theta + PI/2) / PI;
//     return vec2(u, v);
// }

// bool hit_sphere(const Ray &r, const Sphere &s, HitInfo &i, double t_min,
// double t_max)
//     {
//         //sphere intersection code
//         vec3 oc = r.origin - s.position;
//         float a = length_squared(r.direction);
//         float half_b = dot(oc, r.direction);
//         float c = length_squared(oc) - s.radius * s.radius;
//         float discriminant = half_b * half_b - a * c;

//         if (discriminant > 0)
//         {
//             auto root = sqrt(discriminant);
//             auto temp = (-half_b - root) / a;
//             if (temp < t_max && temp > t_min)
//             {
//                 i.t = temp;
//                 i.point = ray_at(r, i.t);
//                 i.normal = (i.point - s.position) / s.radius;

//                 //is a outward-facing face?
//                 bool outwards = dot(r.direction, i.normal) < 0;

//                 vec2 uv = get_sphere_uv((i.point - s.position)/s.radius);
//                 i.u = uv.x;
//                 i.v = uv.y;

//                 return true;
//             }
//             temp = (-half_b + root) / a;
//             if (temp < t_max && temp > t_min)
//             {
//                 i.t = temp;
//                 i.point = ray_at(r, i.t);
//                 i.normal = (i.point - s.position) / s.radius;

//                 //is a outward-facing face?
//                 bool outwards = dot(r.direction, i.normal) < 0;

//                 vec2 uv = get_sphere_uv((i.point - s.position)/s.radius);
//                 i.u = uv.x;
//                 i.v = uv.y;

//                 return true;
//             }
//         }
//         return false;
//     }

// bool intersects_any_sphere(const Ray &r, const std::vector<Sphere> &spheres,
// HitInfo &i)
// {
//     float t_nearest = LARGE_FLOAT;
//     bool t_hit = false;
//     HitInfo hinfo;

//     for (int index = 0; index < spheres.size(); index++)
//     {
//         HitInfo hinfo_temp;

//         if (hit_sphere(r, spheres[index], hinfo_temp) && hinfo_temp.t <
//         t_nearest)
//         {
//             //a closer sphere intersected the ray!
//             t_nearest = hinfo_temp.t;
//             t_hit = true;

//             hinfo = hinfo_temp;
//             hinfo.index = index;
//         }
//     }

//     if (t_hit)
//     {
//         i = hinfo;
//         return true;
//     }
//     return false;
// }

// bool scatter_diffuse(const Ray &r, const HitInfo &h, const Material &m, vec3
// &attenuation, Ray &scattered, RandomState &rnd)
// {
//     // if(m.material_type != MATERIAL_DIFFUSE)
//     //     return false;

//     vec3 target = h.normal + rand_unit_vector(rnd);

//     scattered.origin = h.point;
//     scattered.direction = target;

//     attenuation = m.albedo_colour;
//     //attenuation = vec3(1.0, 0.0, 0.0);

//     return true;
// }

// bool scatter_metal(const Ray &r, const HitInfo &h, const Material &m, vec3
// &attentuation, Ray &scattered, RandomState &rnd)
// {
//     // if(m.material_type != MATERIAL_METAL)
//     //     return false;

//     vec3 reflected = reflect(normalize(r.direction), h.normal);

//     scattered.origin = h.point;
//     scattered.direction = reflected + m.metal_fuzz * rand_unit_vector(rnd);
//     //scattered.direction = reflected;

//     attentuation = m.albedo_colour;

//     return (dot(scattered.direction, h.normal) > 0);
// }

// } //namespace fightingengine