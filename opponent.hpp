#pragma once

#include "player.hpp"
#include "grid_with_sectors.hpp"

class Opponent : public Player_info
{
public:
    using Mark_map = Grid_with_sectors<int16_t>;

    explicit Opponent(Game& game)
        : Player_info(game)
    {}

    void init();

    inline bool sector_is_known() const { return sector >= 0; }
    void reset_sector() { sector = -1; }

    bool silence_used = false;

    void treat_order(const std::string_view& order);

    void update_data_with_orders(const std::string& orders);

    void update_data_with_sonar_result(int sector, bool found);

    int most_marked_sector() const;

    const Mark_map& mark_map() const { return mark_map_; }
    Mark_map& mark_map() { return mark_map_; }

private:
    std::vector<Position> silence_destinations_(Position origin, Direction dir);
    void update_pos_info_with_last_orientation_();
    void update_pos_info_with_sector_();
    void update_pos_info_with_move_dir_(Direction dir);

public:
    int sector = -1;
    std::vector<Direction> relative_path;
    Mark_map mark_map_;
};
