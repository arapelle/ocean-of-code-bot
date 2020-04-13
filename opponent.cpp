#include "opponent.hpp"
#include "game.hpp"
#include "map.hpp"

void Opponent::treat_order(const std::string_view& order)
{
    silence_used = false;

    std::istringstream iss(std::string(order.begin(), order.end()));
    std::string command;
    iss >> command;
    if (command == "SURFACE")
    {
        iss >> sector;
        relative_path.clear();
        update_pos_info_with_sector_();
    }
    else if (command == "MOVE")
    {
        char dir_ch;
        iss >> dir_ch;
        Direction dir = char_to_dir(dir_ch);
        relative_path.push_back(dir);
        update_pos_info_with_move_dir_(dir);
    }
    else if (command == "SILENCE")
    {
        update_pos_info_with_last_orientation_();
        relative_path.push_back(Undefined);
        silence_used = true;
    }
}

void Opponent::update_data_with_orders(const std::string& orders)
{
    std::size_t index = 0;
    std::size_t end_index = 0;
    for (; index < orders.length(); index = end_index + 1)
    {
        end_index = orders.find('|', index);
        end_index = std::min(end_index, orders.length());
        std::string_view order(&orders[index], end_index - index);
        if (!order.empty())
            treat_order(order);
    }
}

//-----

void Opponent::init()
{
    const Map& map = game().map();

    mark_map_.set_sector_size(map.sector_width(), map.sector_height());
    mark_map_.resize(map.width(), map.height(), 0);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            if (!map.get(i,j).is_ocean())
                mark_map_.get(i,j) = -2;
        }
    }
}

std::vector<Position> Opponent::silence_destinations_(Position origin, Direction orientation)
{
    const Map& map = game().map();

    Direction opposed_dir = opposed_direction(orientation);
    std::vector<Position> sdests;
    sdests.push_back(origin);
    for (unsigned i = 0; i < number_of_directions(); ++i)
    {
        Direction dir = Direction(i);
        if (dir != opposed_dir)
        {
            Position npos = origin;
            for (unsigned j = 1; j <= 4; ++j)
            {
                npos.move(dir);
                if (!map.contains(npos))
                    break;
                const Square& square = map.get(npos);
                if (square.is_ocean() && !square.is_visited(id))
                    sdests.push_back(npos);
                else
                    break;
            }
        }
    }
    return sdests;
}

void Opponent::update_pos_info_with_last_orientation_()
{
    trace();
    const Map& map = game().map();
    int previous_turn_number = game().turn_number() - 1;
    Direction last_dir = relative_path.back();

    unsigned mark_count = 0;
    Position last_pos;
    Mark_map next_mark_map(mark_map_);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            Position pos(i,j);
            if (mark_map_.get(pos) == previous_turn_number)
            {
                std::vector<Position> sdests = silence_destinations_(pos, last_dir);
                for (const Position& npos : sdests)
                {
                    next_mark_map.get(npos) = game().turn_number();
                    ++mark_count;
                    last_pos = npos;
                }
            }
        }
    }
//    debug() << mark_map_ << std::endl;
//    debug() << next_mark_map << std::endl;
    mark_map_ = next_mark_map;
    //TODO if mark_count > 1 && all marked squares are in the same sector:
    //         sector = visited_sector;

    if (mark_count == 1)
    {
        position() = last_pos;
        sector = map.position_to_sector_index(last_pos);
    }
}

void Opponent::update_pos_info_with_sector_()
{
    trace();
    const Map& map = game().map();
    int previous_turn_number = game().turn_number() - 1;

    Position origin = map.sector_origin(sector);
    unsigned mark_count = 0;
    Position last_pos;
    for (int j = 0; j < map.sector_height(); ++j)
    {
        for (int i = 0; i < map.sector_width(); ++i)
        {
            Position pos = Position(i,j) + origin;
            if (mark_map_.get(pos) == previous_turn_number)
            {
                mark_map_.get(pos) = game().turn_number();
                ++mark_count;
                last_pos = pos;
            }
        }
    }
    if (mark_count == 1)
        position() = last_pos;
}

void Opponent::update_pos_info_with_move_dir_(Direction dir)
{
    const Map& map = game().map();
    int previous_turn_number = game().turn_number() - 1;

    unsigned mark_count = 0;
    Position last_pos;
    Mark_map next_mark_map(mark_map_);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            Position pos(i,j);
            Position npos = pos.neighbour(dir);
            if (mark_map_.get(pos) == previous_turn_number && map.contains(npos) && map.get(npos).is_ocean())
            {
                next_mark_map.get(npos) = game().turn_number();
                ++mark_count;
                last_pos = npos;
            }
        }
    }
    mark_map_ = next_mark_map;
    //TODO if mark_count > 1 && all marked squares are in the same sector:
    //         sector = visited_sector;

    if (mark_count == 1)
    {
        position() = last_pos;
        sector = map.position_to_sector_index(last_pos);
    }
}

int Opponent::most_marked_sector() const
{
    int res_sector = 1;
    std::size_t count = mark_map_.count_in_sector(res_sector, game().turn_number());
    for (int sector = 1; sector <= mark_map_.number_of_sectors(); ++sector)
    {
        std::size_t cnt = mark_map_.count_in_sector(sector, game().turn_number());
        if (cnt > count)
        {
            count = cnt;
            res_sector = sector;
        }
    }
    return res_sector;
}
