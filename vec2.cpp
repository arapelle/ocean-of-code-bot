#include "vec2.hpp"
#include "log.hpp"

Vec2::Vec2(int x, int y)
    : x(x), y(y)
{}

Vec2& Vec2::operator+=(const Vec2& rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vec2 operator+(const Vec2& lfs, const Vec2& rhs)
{
    Vec2 res = lfs;
    res += rhs;
    return res;
}

Vec2& Vec2::operator-=(const Vec2& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

Vec2 operator-(const Vec2& lfs, const Vec2& rhs)
{
    Vec2 res = lfs;
    res -= rhs;
    return res;
}

bool Vec2::operator==(const Vec2& rfs) const
{
    return x == rfs.x && y == rfs.y;
}

bool Vec2::operator!=(const Vec2& rfs) const
{
    return x != rfs.x && y != rfs.y;
}

Vec2& Vec2::move(Direction dir)
{
    switch (dir)
    {
    case North: --y; break;
    case East: ++x; break;
    case South: ++y; break;
    case West: --x; break;
    default: error() << "Bad Move" << std::endl;
    }
    return *this;
}

Vec2 Vec2::neighbour(Direction dir) const { Vec2 vec(*this); vec.move(dir); return vec; }

std::ostream& operator<<(std::ostream& stream, const Vec2& vec)
{
    return stream << vec.x << " " << vec.y;
}
