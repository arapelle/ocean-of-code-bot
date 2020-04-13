#pragma once

#include "square.hpp"
#include "grid_with_sectors.hpp"

class Map : public Grid_with_sectors<Square>
{
public:
    Map(int width = 0, int height = 0);

    void fill_from_stream(std::istream& stream);

    void clear_visit(int actor_id);

    std::size_t accessibility(const Position& pos, int actor_id) const;

    std::size_t number_of_reachable_squares(const Position& pos, int actor_id);

    friend std::ostream& operator<<(std::ostream& stream, const Map& map);
};
