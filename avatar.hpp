#pragma once

#include "player.hpp"
#include "tool.hpp"

class Avatar : public Player
{
public:
    struct Toolkit
    {
        Torpedo torpedo;
        Sonar sonar;
        Silence silence;
        Mine mine;

        explicit Toolkit(Player& player)
            : torpedo(player), sonar(player), silence(player), mine(player)
        {}
    };

public:
    explicit Avatar(Game& game)
        : Player(game), toolkit(*this)
    {}

    Toolkit toolkit;
    const Torpedo& torpedo() const { return toolkit.torpedo; }
    Torpedo& torpedo() { return toolkit.torpedo; }
    const Sonar& sonar() const { return toolkit.sonar; }
    Sonar& sonar() { return toolkit.sonar; }
    const Silence& silence() const { return toolkit.silence; }
    Silence& silence() { return toolkit.silence; }
    const Mine& mine() const { return toolkit.mine; }
    Mine& mine() { return toolkit.mine; }

    bool has_lost_life() const;
};
