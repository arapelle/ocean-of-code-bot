#pragma once

#include "grid.hpp"
#include <deque>
#include <cassert>

class Game;

class Player_info
{
public:
    inline static constexpr std::size_t max_history_size = 5;

    struct Status
    {
        Position position = Position(-1, -1);
        int hp = -1;
    };

    Player_info()
    {}

    explicit Player_info(Game& game)
        : game_(&game)
    {}

    const Position& position() const { return status.position; }
    Position& position() { return status.position; }
    bool position_is_known() const { return position().x >= 0 && position().y >= 0; }
    const int& hp() const { return status.hp; }
    int& hp() { return status.hp; }

    const Status& previous_status() const { return history_status.back(); }
    void save_status()
    {
        while (history_status.size() >= max_history_size)
            history_status.pop_front();
        history_status.push_back(status);
    }

    const Game& game() const { assert(game_); return *game_; }
    Game& game() { assert(game_); return *game_; }

    friend std::istream& operator>>(std::istream& stream, Player_info& info)
    {
        return stream >> info.id;
    }

private:
    Game* game_ = nullptr;

public:
    int id = -1;
    Status status;
    std::deque<Status> history_status;
};
