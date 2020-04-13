#pragma once

#include "player.hpp"
#include "tool.hpp"

class Avatar : public Player_info
{
public:
    struct Toolkit
    {
        Torpedo torpedo;
        Sonar sonar;
        Silence silence;
        Mine mine;

        explicit Toolkit(Player_info& player)
            : torpedo(player), sonar(player), silence(player), mine(player)
        {}
    };

public:
    explicit Avatar(Game& game)
        : Player_info(game), toolkit(*this)
    {}

    Toolkit toolkit;
    const Tool& torpedo() const { return toolkit.torpedo; }
    Tool& torpedo() { return toolkit.torpedo; }
    const Sonar& sonar() const { return toolkit.sonar; }
    Sonar& sonar() { return toolkit.sonar; }
    const Tool& silence() const { return toolkit.silence; }
    Tool& silence() { return toolkit.silence; }
    const Tool& mine() const { return toolkit.mine; }
    Tool& mine() { return toolkit.mine; }
};
