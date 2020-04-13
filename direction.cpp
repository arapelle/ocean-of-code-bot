#include "direction.hpp"
#include "log.hpp"

Direction char_to_dir(char ch)
{
    switch (ch)
    {
    case 'N': return North;
    case 'E': return East;
    case 'S': return South;
    case 'W': return West;
    default: error() << "Bad char dir" << std::endl;
        ;
    }
    return Bad;
}

std::string dir_to_string(Direction dir)
{
    switch (dir)
    {
    case North: return "N";
    case East: return "E";
    case South: return "S";
    case West: return "W";
    case Undefined: return "?";
    case Bad: return "%";
    }
    return "ERR: @_@";
}
