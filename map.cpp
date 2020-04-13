#include "map.hpp"
#include <queue>

Map::Map(int width, int height)
    : Grid_with_sectors<Square>(width, height)
{}

void Map::fill_from_stream(std::istream& stream)
{
    std::string line;
    for (auto& row : data_)
    {
        std::getline(stream, line);
        for (int i = 0; i < width_; ++i)
            row[i] = Square(line.at(i));
    }
}

void Map::clear_visit(int actor_id)
{
    for (auto& row : data_)
        for (auto& square : row)
            square.unset_visited(actor_id);
}

std::size_t Map::accessibility(const Position& pos, int actor_id) const
{
    std::size_t res = 0;
    const Square& square = get(pos);
    if (square.is_ocean() && !square.is_visited(actor_id))
    {
        for (unsigned dir = 0; dir < number_of_directions(); ++dir)
        {
            Position npos = pos.neighbour(Direction(dir));
            if (contains(npos))
            {
                const Square& nsquare = get(npos);
                if (!nsquare.is_visited(actor_id) && nsquare.is_ocean())
                    ++res;
            }
        }
    }
    return res;
}

std::size_t Map::number_of_reachable_squares(const Position& pos, int actor_id)
{
    std::size_t count = 0;

    Grid<uint8_t> visited(width(), height(), 0);
    std::queue<Position> posq;
    auto is_reachable = [&](const Position& sqpos)
    {
        const Square& square = get(sqpos);
        return square.is_ocean() && !square.is_visited(actor_id);
    };
    auto visit = [&](const Position& sqpos)
    {
        visited.get(sqpos) = 1;
        posq.push(sqpos);
        ++count;
    };

    if (is_reachable(pos))
        visit(pos);

    while (!posq.empty())
    {
        Position cpos = posq.front();
        posq.pop();
        for (unsigned i = 0; i < number_of_directions(); ++i)
        {
            Direction dir = Direction(i);
            Position npos = cpos.neighbour(dir);
            if (contains(npos) && is_reachable(npos) && !visited.get(npos))
                visit(npos);
        }
    }

    return count;
}

std::ostream& operator<<(std::ostream& stream, const Map& map)
{
    for (const auto& row : map.data_)
    {
        for (const auto& square : row)
            stream << square.type();
        stream << std::endl;
    }
    return stream;
}
