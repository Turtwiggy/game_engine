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

uniform int set_triangles;

layout( std430, binding = 2 ) readonly buffer bufferData
{
    triangle triangles[];
};

uniform vec3 eye, ray00, ray01, ray10, ray11;

#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI (1.0 / PI)
#define ONE_OVER_2PI (1.0 / TWO_PI)
#define LARGE_FLOAT 1E+10
#define EPSILON 0.0000001
#define LIGHT_INTENSITY 1.0
#define SKY_COLOUR vec3(0.2, 0.3, 1.0)
#define MAX_SCENE_BOUNDS 100.0

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
};

bool intersects(ray r, triangle tri, out hitinfo i)
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

    bool culling = false;

    if(culling) {
        if(det < EPSILON) {
            return false;
        }
    } else {
        if(abs(det) < EPSILON) {
            return false;
        }
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
        //i.t = (orig + dir * t);

        i.t = t;
        return true;
    }

    return false; // this ray hits the triangle 
}

//https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
vec3 trace(ray r, vec3 normal) {

    vec3 origin = r.origin;
    vec3 att = vec3(1.0);

    const float bounces = 1;

    //use texture information for first bounce
    //if(dot(r.direction, normal) == 0.0) {
    //    //did not hit anything!
    //    return LIGHT_INTENSITY * SKY_COLOUR * att;
    //}
    // the ray hit something!
    //vec3 visulized_normal = 0.5 * vec3(normal.x + 1, normal.y + 1, normal.z + 1);
    //return visulized_normal;

    float t_nearest = LARGE_FLOAT;
    triangle tri_nearest;
    bool intersect = false;

    for(int i = 0; i < set_triangles; i++) {

        const triangle tri = triangles[i];
        hitinfo h;

        if(intersects(r, tri, h) && h.t < t_nearest)
        {
            //a closer triangle intersected the ray!
            tri_nearest = tri;
            t_nearest = h.t;
            intersect = true;
        }
    }

    if(intersect)
    {
        //return the triangles colour!
        //return tri_nearest.p0.colour.xyz;
        return vec3(0.2, 0.6, 0.2);
    }

    for(int bounce = 0; bounce < bounces; bounce++) {
        //get the point of intersection
        //vec3 point = origin + hinfo.near * dir;
        //vec3 normal = hinfo.normal;
        //offset point small amount by surface normal
        //origin = point + normal * EPSILON;
        //evaluate the surface BRDF
        //vec3 dir = randomHemispherePoint(normal, randvec2(bounce));
        //att *= ONE_OVER_PI;
        //att *= dot(dir, normal);
        //colour of material (albedo)
        //att *= b.col;   
        //att *= vec3(1.0, 0.1, 0.1);
        //att /= ONE_OVER_2PI;
    }

    // bool frontface;
    // //ray is from inside normal
    return LIGHT_INTENSITY * SKY_COLOUR * att;
}


layout (local_size_x = 16, local_size_y = 8) in;
void main(void) {

    ivec2 px = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(outTexture);

    if (any(greaterThanEqual(px, size)))
        return;

    vec2 p = (vec2(px) + vec2(0.5)) / vec2(size);
    vec3 dir = mix(mix(ray00, ray01, p.y), mix(ray10, ray11, p.y), p.x);
    ray fwd = {eye, dir};

    //Sample textures
    //vec3 FragPos = imageLoad(positionData, px).rgb;
    vec3 Normal = imageLoad(normalTexture, px).rgb;

    vec3 color = trace(fwd, Normal);

    imageStore(outTexture, px, vec4(color, 1.0));
}
