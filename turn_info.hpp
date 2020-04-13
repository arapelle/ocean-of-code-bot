#pragma once

#include <string>
#include <istream>
#include <ostream>

struct Turn_info
{
    // Self info
    int x = -1;
    int y = -1;
    int myLife = -1;
    int oppLife = -1;
    int torpedoCooldown = -1;
    int sonarCooldown = -1;
    int silenceCooldown = -1;
    int mineCooldown = -1;
    // Sonar info
    std::string sonarResult = "?";
    // Opponent info
    std::string opponentOrders = "?";

    friend std::ostream& operator<<(std::ostream& stream, const Turn_info& info);

    friend std::istream& operator>>(std::istream& stream, Turn_info& info);
};
