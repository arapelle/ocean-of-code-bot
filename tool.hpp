#pragma once

#include "grid.hpp"
#include <string_view>
#include <cstdint>
#include <cassert>

class Game;
class Player;

struct Tool
{
    explicit Tool(Player& player, int total_cooldown)
        : player_(&player), total_cooldown_(total_cooldown)
    {}
    bool is_available() const { return cooldown_ >= 0; }
    bool is_ready() const { return cooldown_ == 0; }
    std::size_t number_of_loads() const { return total_cooldown_ - cooldown_; }
    std::size_t cooldown() const { return cooldown_; }
    void set_cooldown(int cooldown) { cooldown_ = cooldown; }
    const Player& player() const { assert(player_); return *player_; }
    Player& player() { assert(player_); return *player_; }

private:
    Player* player_ = nullptr;
    int total_cooldown_ = -1;
    int cooldown_ = -1;
};

struct Sonar : public Tool
{
    inline static constexpr int total_cooldown() { return 4; }
    inline static constexpr std::string_view result_not_available() { return "NA"; }
    inline static constexpr std::string_view result_opponent_found() { return "Y"; }
    inline static constexpr std::string_view result_opponent_not_found() { return "N"; }

    explicit Sonar(Player& player) : Tool(player, total_cooldown()) {}

    int requested_sector() const { return requested_sector_; }
    void set_request(int sector)
    {
        requested_sector_ = sector;
//        request_answer_ = false;
    }
    void reset_request() { set_request(-1); }
    void update_info(const std::string& sonar_result);

private:
    int requested_sector_ = -1;
//    bool request_answer_ = false;
};

struct Torpedo : public Tool
{
public:
    inline static constexpr int total_cooldown() { return 3; }
    inline static constexpr int max_radius() { return 4; }

    explicit Torpedo(Player& player) : Tool(player, total_cooldown()) {}
    void fire_to(Position targeted_position)
    {
        targeted_position_ = targeted_position;
    }
    void reset_targeted_position() { targeted_position_ = Position(-1,-1); }
    void update_info();

private:
    Position targeted_position_ = Position(-1,-1);
};

struct Silence : public Tool
{
public:
    inline static constexpr int total_cooldown() { return 6; }

    explicit Silence(Player& player) : Tool(player, total_cooldown()) {}

private:
    //TODO
};

struct Mine : public Tool
{
public:
    inline static constexpr int total_cooldown() { return 3; }

    explicit Mine(Player& player) : Tool(player, total_cooldown()) {}

private:
    //TODO
};
