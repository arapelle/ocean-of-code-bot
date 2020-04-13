#pragma once

#include <cstdint>
#include <string>

enum Direction : char
{
    North,
    East,
    South,
    West,
    Undefined = '?',
    Bad = '%',
};


inline std::size_t number_of_directions() { return 4; }

inline bool dir_is_valid(Direction dir) { return unsigned(dir) < number_of_directions(); }

inline Direction opposed_direction(Direction dir) { return dir_is_valid(dir) ? Direction((unsigned(dir) + 2) % 4) : dir; }

Direction char_to_dir(char ch);

std::string dir_to_string(Direction dir);
