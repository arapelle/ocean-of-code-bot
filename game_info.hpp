#pragma once

#include <istream>

struct Game_info
{
    int map_width = -1;
    int map_height = -1;

    inline friend std::istream& operator>>(std::istream& stream, Game_info& info)
    {
        return stream >> info.map_width >> info.map_height;
    }
};
