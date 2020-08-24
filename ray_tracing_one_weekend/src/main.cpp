
#include "glm/glm.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>

int main()
{
    printf("beginning... \n");

    std::ofstream myfile;

    const char* filename = "myimage.ppm";
    myfile.open(filename);

    int width = 200;
    int height = 100;
    myfile << "P3\n" << width << " " << height << "\n255\n";

    glm::vec3 lower_left_corner(-2.0, -1.0, -1.0);
    glm::vec3 horizontal(4.0, 0.0, 0.0);
    glm::vec3 vertical(0.0, 2.0, 0.0);
    glm::vec3 origin(0.0, 0.0, 0.0);

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            //Draw in rows across
            //std::cout << "x: " << x << " y: " << y << "\n";

            float u = float(x) / float(width);
            float v = float(y) / float(height);

            //ray r = cam.get_ray(u, v);
            //glm::vec3 p = r.point_at_parameter(2.0);
            //glm::vec3 col = color(r, world);
            //int ir = int(255.99 * col[0]);
            //int ig = int(255.99 * col[1]);
            //int ib = int(255.99 * col[2]);

            auto r = double(x) / (width - 1);
            auto g = double(y) / (height - 1);
            auto b = 1;

            int ir = static_cast<int>(255.999 * r);
            int ig = static_cast<int>(255.999 * g);
            int ib = static_cast<int>(255.999 * b);

            myfile << ir << " " << ig << " " << ib << "\n";
        }
    }

    myfile.close();
    printf("done! \n");
    return 0;
}
