
//header
#include "breakout/game.hpp"

//C++ lib headers
#include <iostream>
#include <fstream>
#include <sstream>

//your lib headers
#include "engine/resources/resource_manager.hpp"

namespace game2d
{

void load_level_from_file(std::vector<std::vector<int>>& layout, const std::string &path)
{
    std::ifstream fstream(path);

    if (fstream)
    {
        int tile_code;
        std::string line;
        while (std::getline(fstream, line))
        {
            //std::cout << line << "\n";

            std::istringstream sstream(line);
            std::vector<int> row;

            while (sstream >> tile_code)
            {
                //std::cout << tile_code << "\n";

                row.push_back(tile_code);
            }

            if(row.size() > 0)
                layout.push_back(row);
        }
    }
    else
    {
        std::cout << "error reading game level file! \n";
    }
};

void init_level(GameLevel& level, const std::vector<std::vector<int>>& layout, int level_width, int level_height)
{
    level.bricks.clear();

    int height = static_cast<int>(layout.size());
    int width = static_cast<int>(layout[0].size());
    //printf("level, h: %i w: %i \n", height, width);

    float unit_width = level_width / static_cast<float>(width);
    float unit_height = level_height / height; 

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int tile_data = layout[y][x];

            glm::vec2 pos(unit_width * x, unit_height * y);
            glm::vec2 size(unit_width, unit_height);
            //printf("tile_data: %i pos x: %f y: %f size x: %f y: %f \n", tile_data, pos.x, pos.y, size.x, size.y);
            
            GameObject go;

            if( tile_data == 1 ) {
                go.texture = fightingengine::ResourceManager::get_texture("block_solid");
                //transform
                go.transform.position = pos;
                go.transform.scale = size;
                go.transform.colour = {0.8f, 0.8f, 0.7f};
                go.transform.angle = 0.0f;
                //state
                go.is_solid = true;
                go.destroyed = false;
            } else
            {
                go.texture = fightingengine::ResourceManager::get_texture("block");

                glm::vec3 color = glm::vec3(1.0f); // original: white
                if (tile_data == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f); //blueish
                else if (tile_data == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f); //greenish
                else if (tile_data == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f); 
                else if (tile_data == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                //transform
                go.transform.position = pos;
                go.transform.scale = size;
                go.transform.colour = color;
                go.transform.angle = 0.0f;
                //state
                go.is_solid = false;
                go.destroyed = false;
            }

            level.bricks.push_back(go);
        }
    }
}

} //namespace game2d
