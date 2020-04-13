#pragma once

#include "direction.hpp"

class Vec2
{
public:
    int x = 0;
    int y = 0;

    Vec2() = default;
    Vec2(int x, int y);

    Vec2& operator+=(const Vec2& rhs);

    friend Vec2 operator+(const Vec2& lfs, const Vec2& rhs);

    Vec2& operator-=(const Vec2& rhs);

    friend Vec2 operator-(const Vec2& lfs, const Vec2& rhs);

    bool operator==(const Vec2& rfs) const;

    bool operator!=(const Vec2& rfs) const;

    Vec2& move(Direction dir);

    Vec2 neighbour(Direction dir) const;

    friend std::ostream& operator<<(std::ostream& stream, const Vec2& vec);

    std::string to_string() const;
};
