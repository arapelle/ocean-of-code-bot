#include "tool.hpp"
#include "log.hpp"
#include <string>
#include <algorithm>

#include "game.hpp"
#include "opponent.hpp"

void Sonar::update_info(const std::string& sonar_result)
{
//    trace();
    if (sonar_result != result_not_available())
    {
//        trace();
        Game& game = player().game();
        Opponent& opponent = game.opponent();
        Opponent::Mark_map& mark_map = opponent.mark_map();
        int previous_mark = opponent.relative_path.size();
//        debug() << mark_map;

        bool opponent_is_present = sonar_result == result_opponent_found();
        if (opponent_is_present)
        {
            int sector_count = mark_map.number_of_sectors();
            for (int sector = 1; sector <= sector_count; ++sector)
            {
                if (sector != requested_sector_)
                    mark_map.fill_sector_if(sector, -1, previous_mark);
            }
        }
        else
            mark_map.fill_sector_if(requested_sector_, -1, previous_mark);
//        debug() << mark_map;
    }
    reset_request();
}

void Torpedo::update_info()
{
//    trace();
    Game& game = player().game();
    const Map& map = game.map();
    if (!map.contains(targeted_position_))
        return;

    Opponent& opponent = game.opponent();
    assert(!opponent.history_status.empty());

    Opponent::Mark_map& mark_map = opponent.mark_map();
    int previous_mark = opponent.relative_path.size();
    int diff_hp = opponent.previous_status().hp - opponent.hp();
    switch (diff_hp)
    {
    case 0:
    {
        std::vector<Position> targeted_positions = targeted_position_.square_area(1);
        for (const Position& pos : targeted_positions)
            if (map.contains(pos) && map.get(pos).is_ocean())
                mark_map.get(pos) = -7;
        break;
    }
    case 1:
    {
        std::vector<Position> targeted_positions = targeted_position_.square_area(1);
//        targeted_positions.erase(std::find(targeted_positions.begin(), targeted_positions.end(), targeted_position_));
        for (int j = 0; j < mark_map.height(); ++j)
        {
            for (int i = 0; i < mark_map.width(); ++i)
            {
                Position pos(i,j);
                if (map.get(pos).is_ocean() && std::find(targeted_positions.begin(), targeted_positions.end(), pos) == targeted_positions.end())
                    mark_map.get(pos) = -8;
            }
        }
        mark_map.get(targeted_position_) = -8;
        break;
    }
    case 2:
    {
        for (int j = 0; j < mark_map.height(); ++j)
        {
            for (int i = 0; i < mark_map.width(); ++i)
            {
                Position pos(i,j);
                if (map.get(pos).is_ocean())
                    mark_map.get(pos) = -9;
            }
        }
        mark_map.get(targeted_position_) = previous_mark;
        break;
    }
    default:
        error() << "WOOT?!" << std::endl;
    }

    reset_targeted_position();
}
