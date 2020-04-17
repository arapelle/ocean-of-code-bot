#pragma once

#include "opponent.hpp"
#include "avatar.hpp"
#include "map.hpp"
#include "turn_info.hpp"
#include "game_info.hpp"
#include "grid.hpp"
#include <istream>
#include <ostream>

class Game
{
public:
    static int default_sector_width() { return 5; }
    static int default_sector_height() { return 5; }

    Game(std::istream& istream, std::ostream& ostream)
         : avatar_(*this), opponent_(*this),
           istrm_(istream), ostrm_(ostream)
    {}

    // START
    void init();

    void print_start_info() const;

    Position choose_start_position();

    void play_start_actions();

    // TURN
    // update:
    void update_data(const Turn_info& turn_info);

    // ia:
    Direction move_direction();
    Direction exploration_move_direction();
    Direction move_to_opponent_direction();

    void do_actions();

    void do_main_actions();

    std::string_view load_submarine_tool();

    // actions formatting:
    std::string silence_action(Direction dir, unsigned distance);

    std::string move_action(Direction dir);

    // play turn:
    void play_turn();

    // MISC
    const Game_info& game_info() const { return game_info_; }
    const Map& map() const { return map_; }
    Map& map() { return map_; }
    const Avatar& avatar() const { return avatar_; }
    const Opponent& opponent() const { return opponent_; }
    Opponent& opponent() { return opponent_; }
    int turn_number() const { return turn_number_; }

private:
    int turn_number_ = 0;
    Game_info game_info_;
    Map map_;
    Avatar avatar_;
    Opponent opponent_;

    std::istream& istrm_;
    std::ostream& ostrm_;
};
