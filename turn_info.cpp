#include "turn_info.hpp"

std::ostream& operator<<(std::ostream& stream, const Turn_info& info)
{
    stream << "TURN INFO:\n{" << std::endl;
    stream << "pos: (" << info.x << " " << info.y << "), " << "HP: " << info.myLife << ",\n";
    if (info.torpedoCooldown >= 0)
        stream << "torpedo_cooldown: " << info.torpedoCooldown << "\n";
    if (info.sonarCooldown >= 0)
    {
        stream << "sonar_cooldown: " << info.sonarCooldown << "\n";
        stream << "sonar: " << info.sonarResult << "\n";
    }
    if (info.silenceCooldown >= 0)
        stream << "silence_cooldown: " << info.silenceCooldown << "\n";
    if (info.mineCooldown >= 0)
        stream << "mine_cooldown: " << info.mineCooldown << "\n";
    stream << "Opponent's HP: " << info.oppLife << "\n"
           << "Opponent's Orders: " << info.opponentOrders << "\n}";
    return stream;
}

std::istream& operator>>(std::istream& stream, Turn_info& info)
{
    stream >> info.x >> info.y >> info.myLife >> info.oppLife >> info.torpedoCooldown
           >> info.sonarCooldown >> info.silenceCooldown >> info.mineCooldown;
    stream.ignore();
    stream >> info.sonarResult;
    stream.ignore();
    std::getline(stream, info.opponentOrders);
    return stream;
}
