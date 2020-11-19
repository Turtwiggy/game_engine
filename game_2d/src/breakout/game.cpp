
//header
#include "breakout/game.hpp"

//C++ lib headers
#include <iostream>  
#include <fstream>      
#include <sstream>

namespace game2d {

void load_level_from_file(std::vector<std::vector<int>>& layout, const std::string& path) 
{
    std::ifstream fstream(path);

    if (fstream)
    {
        int tile_code;
        std::string line;
        while(std::getline(fstream, line))
        {   
            std::cout << line << "\n";

            std::istringstream sstream(line);
            std::vector<int> row;

            while (sstream >> tile_code)
            {
               std::cout << tile_code << "\n";
               row.push_back(tile_code);
            }
            layout.push_back(row);
        }
    } 
    else
    {
        std::cout << "error reading game level file! \n";
    }
};

void init_level(GameLevel& level, const std::vector<std::vector<int>>& layout, int width, int height)
{
    int h = layout.size();
    int w  = layout[0].size();

    float unit_width  = width / static_cast<float>(w);
    float unit_height = height / static_cast<float>(h);

    for( int y = 0; y < height; y++)
    {
        for( int x = 0; x < width; x++) 
        {
            //TODO(Turtwiggy): finish this
        }
    }

} //namespace game2d
