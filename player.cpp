#include "player.hpp"
#include "tool.hpp"
#include "game.hpp"
#include <algorithm>

Player::Player()
{}

Player::Player(Game& game)
    : game_(&game)
{}

void Player::save_status()
{
    while (history_status.size() >= max_history_size)
        history_status.pop_front();
    history_status.push_back(status);
}

std::vector<Position> Player::sensible_squares() const
{
    std::vector<Position> res;
    if (position_is_known())
    {
        const Map& map = game().map();
        std::vector<Position> possible_positions = position().square_area(1);

        for (const Position& pos : possible_positions)
            if (map.contains(pos) && map.get(pos).is_ocean())
                res.push_back(pos);
    }
    return res;
}

std::vector<Position> Player::hitable_squares_by_torpedo() const
{
    const Map& map = game().map();
    std::vector<Position> rvpos = map.reachable_squares(position(), Torpedo::max_radius());
    std::vector<Position> svpos = sensible_squares();
    std::sort(rvpos.begin(), rvpos.end());
    std::sort(svpos.begin(), svpos.end());
    std::vector<Position> vpos;
    const Opponent& opponent = game().opponent();
    if (opponent.position_is_known() && opponent.hp() < hp())
        vpos = std::move(rvpos);
    else
        std::set_difference(rvpos.begin(), rvpos.end(), svpos.begin(), svpos.end(), std::back_inserter(vpos));
    return vpos;
}

std::istream& operator>>(std::istream& stream, Player& info)
{
    return stream >> info.id;
}
