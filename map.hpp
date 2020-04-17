#pragma once

#include "square.hpp"
#include "grid_with_sectors.hpp"
#include <limits>

class Map : public Grid_with_sectors<Square>
{
public:
    Map(int width = 0, int height = 0);

    void fill_from_stream(std::istream& stream);

    void clear_visit(int actor_id);

    std::size_t accessibility(const Position& pos, int actor_id) const;

    std::size_t number_of_reachable_squares(const Position& pos, int actor_id);

    std::vector<Position> reachable_squares(const Position& pos, std::size_t radius = std::numeric_limits<std::size_t>::max()) const;

    Direction dir_to(int avatar_id, const Position& start, const Position& dest) const;

    friend std::ostream& operator<<(std::ostream& stream, const Map& map);
};
