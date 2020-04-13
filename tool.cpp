#include "tool.hpp"
#include "log.hpp"
#include <string>

#include "game.hpp"
#include "opponent.hpp"

void Sonar::manage_info(const std::string& sonar_result)
{
    if (sonar_result != result_not_available())
    {
        trace();
        Game& game = player().game();
        Opponent& opponent = game.opponent();
        Opponent::Mark_map& mark_map = opponent.mark_map();
        int previous_turn_number = game.turn_number() - 1;
//        debug() << mark_map;

        bool opponent_is_present = sonar_result == result_opponent_found();
        if (opponent_is_present)
        {
            int sector_count = mark_map.number_of_sectors();
            for (int sector = 1; sector <= sector_count; ++sector)
            {
                if (sector != requested_sector_)
                    mark_map.fill_sector_if(sector, -1, previous_turn_number);
            }
        }
        else
            mark_map.fill_sector_if(requested_sector_, -1, previous_turn_number);
//        debug() << mark_map;
    }
    reset_request();
}
