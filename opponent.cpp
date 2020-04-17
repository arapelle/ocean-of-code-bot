#include "opponent.hpp"
#include "game.hpp"
#include "map.hpp"
#include <algorithm>

void Opponent::treat_order(const std::string_view& order)
{
    silence_used = false;
    torpedo_used = false;

    std::istringstream iss(std::string(order.begin(), order.end()));
    std::string command;
    iss >> command;
    if (command == "SURFACE")
    {
        iss >> sector;
        update_pos_info_with_sector_();
        relative_path.clear();
        game().map().clear_visit(id);
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
        relative_path.push_back(Undefined);
        update_pos_info_with_last_orientation_();
        silence_used = true;
    }
    else if (command == "TORPEDO")
    {
        int x = -1;
        int y = -1;
        iss >> x >> y;
        update_pos_info_with_torpedo_(x, y);
        torpedo_used = true;
    }
}

void Opponent::update_data_with_orders(const std::string& orders)
{
    trace();
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

void Opponent::update_pos_info_with_torpedo_(int x, int y)
{
    const Game& game = this->game();
    const Map& map = game.map();
    int previous_mark = relative_path.size();

    std::vector<Position> squares = map.reachable_squares(Position(x,y), Torpedo::max_radius());

    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            Position pos(i,j);
            auto& mark = mark_map_.get(pos);
            if (mark == previous_mark && std::find(squares.begin(), squares.end(), pos) == squares.end())
                mark = -1;
        }
    }
}

std::vector<Position> Opponent::silence_destinations_(Position origin, Direction orientation, const std::vector<Position>& prpos) const
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
            Position rnpos(0,0);
            for (unsigned j = 1; j <= 4; ++j)
            {
                rnpos.move(dir);
                Position npos = rnpos + origin;
                if (!map.contains(npos) || std::find(prpos.begin(), prpos.end(), rnpos) != prpos.end())
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
//    trace();
    const Map& map = game().map();
    int previous_mark = relative_path.size() - 1;
    Direction last_dir = relative_path.back();
    std::vector<Position> prpos = previous_relative_positions();

    unsigned mark_count = 0;
    Position last_pos;
    Mark_map next_mark_map(mark_map_);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            Position pos(i,j);
            if (mark_map_.get(pos) == previous_mark)
            {
                std::vector<Position> sdests = silence_destinations_(pos, last_dir, prpos);
                for (const Position& npos : sdests)
                {
                    next_mark_map.get(npos) = relative_path.size();
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
//    trace();
    const Map& map = game().map();
    int previous_mark = relative_path.size();

    Position origin = map.sector_origin(sector);
    unsigned mark_count = 0;
    Position last_pos;
    for (int j = 0; j < map.sector_height(); ++j)
    {
        for (int i = 0; i < map.sector_width(); ++i)
        {
            Position pos = Position(i,j) + origin;
            if (mark_map_.get(pos) == previous_mark)
            {
                mark_map_.get(pos) = 0;
                ++mark_count;
                last_pos = pos;
            }
            else if (map.get(pos).is_ocean())
                mark_map_.get(pos) = -1;
        }
    }
    for (int i = 1; i <= mark_map_.number_of_sectors(); ++i)
        if (i != sector)
            mark_map_.fill_sector_if(i, -1, 0);
    if (mark_count == 1)
        position() = last_pos;
}

void Opponent::update_pos_info_with_move_dir_(Direction dir)
{
//    trace();
    const Map& map = game().map();
    int previous_mark = relative_path.size() - 1;

    unsigned mark_count = 0;
    Position last_pos;
    Mark_map next_mark_map(mark_map_);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            Position pos(i,j);
            Position npos = pos.neighbour(dir);
            if (mark_map_.get(pos) == previous_mark && map.contains(npos) && map.get(npos).is_ocean())
            {
                next_mark_map.get(npos) = relative_path.size();
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
//    trace();
    int res_sector = 1;
    std::size_t count = mark_map_.count_in_sector(res_sector, relative_path.size());
    for (int sector = 1; sector <= mark_map_.number_of_sectors(); ++sector)
    {
        std::size_t cnt = mark_map_.count_in_sector(sector, relative_path.size());
        if (cnt > count)
        {
            count = cnt;
            res_sector = sector;
        }
    }
    return res_sector;
}

std::size_t Opponent::number_of_possible_positions() const
{
    return mark_map_.count(relative_path.size());
}

std::vector<Position> Opponent::previous_relative_positions() const
{
    std::vector<Position> vpos;
    Position pos(0,0);
    for (auto iter = relative_path.rbegin() + 1, end_iter = relative_path.rend(); iter != end_iter; ++iter)
    {
        Direction dir = *iter;
        if (!dir_is_valid(dir))
            break;
        pos.move(opposed_direction(dir));
        vpos.push_back(pos);
    }
    return vpos;
}

void Opponent::update_position()
{
    std::size_t pos_count = number_of_possible_positions();
    if (pos_count == 1)
    {
        for (int j = 0; j < mark_map_.width(); ++j)
        {
            for (int i = 0; i < mark_map_.width(); ++i)
            {
                if (mark_map_.get(i,j) == static_cast<int>(relative_path.size()))
                    position() = Position(i,j);
            }
        }
    }
    else
        position() = Position(-1,-1);
}

Position Opponent::center_of_possible_positions() const
{
//    trace();
    int current_mark = relative_path.size();

    std::vector<Position> vpos;
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            if (mark_map_.get(i,j) == current_mark)
            {
                vpos.push_back(Position(i,j));
                if (vpos.size() > 9)
                    return Position(-1,-1);
            }
        }
    }
    if (vpos.empty())
        return Position(-1,-1); //TODO should not happen
    Position center(0,0);
    for (const Position& pos : vpos)
        center += pos;
    debug() << "vpos_size = " << vpos.size() << std::endl;
    assert(vpos.size());
    center.x /= vpos.size();
    center.y /= vpos.size();
    return center;
}
