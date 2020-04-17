#include "map.hpp"
#include <queue>
#include <cassert>

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

std::vector<Position> Map::reachable_squares(const Position& pos, std::size_t radius) const
{
    std::vector<Position> positions;

    Grid<int16_t> visited(width(), height(), -1);
    std::queue<Position> posq;
    auto is_reachable = [&](const Position& sqpos, int16_t dist)
    {
        const Square& square = get(sqpos);
        return square.is_ocean() && dist <= static_cast<int>(radius);
    };
    auto visit = [&](const Position& sqpos, int16_t dist)
    {
        visited.get(sqpos) = dist;
        posq.push(sqpos);
        positions.push_back(sqpos);
    };

    if (is_reachable(pos, 0))
        visit(pos, 0);

    while (!posq.empty())
    {
        Position cpos = posq.front();
        posq.pop();
        int16_t dist = visited.get(cpos) + 1;
        for (unsigned i = 0; i < number_of_directions(); ++i)
        {
            Direction dir = Direction(i);
            Position npos = cpos.neighbour(dir);
            if (contains(npos) && visited.get(npos) < 0 && is_reachable(npos, dist))
                visit(npos, dist);
        }
    }

    return positions;
}

struct Mark
{
    Position previous_position = Position(-1,-1);
    Direction direction = Undefined;

    Mark() : previous_position(-1,-1), direction(Undefined) {}
    Mark(const Position& ppos, Direction dir) : previous_position(ppos), direction(dir) {}
    Direction opposed_direction() const { return ::opposed_direction(direction); }
    bool is_undefined() const { return direction == Undefined; }

    friend std::ostream& operator<<(std::ostream& stream, const Mark& mark)
    {
        return stream << "[" << mark.previous_position << "," << dir_to_string(mark.direction) << "]";
    }
};

Direction Map::dir_to(int avatar_id, const Position& start, const Position& dest) const
{
    trace();
    Direction dir = Bad;

    Grid<Mark> marks(width(), height());
    std::queue<Position> posq;
    auto is_reachable = [&](const Position& sqpos/*, int16_t dist*/)
    {
        const Square& square = get(sqpos);
        return square.is_ocean() && !square.is_visited(avatar_id) /*&& dist <= static_cast<int>(radius)*/;
    };
    auto visit = [&](const Position& sqpos, const Mark& mark)
    {
        marks.get(sqpos) = mark;
        posq.push(sqpos);
    };

    if (get(start).is_ocean())
        visit(start, Mark(start, Bad));

    while (!posq.empty() && marks.get(dest).is_undefined())
    {
        Position cpos = posq.front();
        posq.pop();
        for (unsigned i = 0; i < number_of_directions(); ++i)
        {
            Direction dir = Direction(i);
            Position npos = cpos.neighbour(dir);
            Mark mark(cpos, dir);
            if (contains(npos) && marks.get(npos).is_undefined() && is_reachable(npos))
                visit(npos, mark);
        }
    }

//    debug() << marks << std::endl;
    const Mark* pmark = &marks.get(dest);
    if (!pmark->is_undefined())
    {
        while (pmark->previous_position != start)
            pmark = &marks.get(pmark->previous_position);
//        debug() << "\nmark final: " << *pmark << ", ";
        dir = pmark->direction;
    }
    return dir;
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
