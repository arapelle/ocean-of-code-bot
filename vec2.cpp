#include "vec2.hpp"
#include "log.hpp"
#include <sstream>

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
    return x != rfs.x || y != rfs.y;
}

bool Vec2::operator<(const Vec2& rfs) const
{
    return x < rfs.x || (x == rfs.x && y < rfs.y);
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

std::vector<Vec2> Vec2::circle_area(unsigned radius) const
{
    std::vector<Vec2> vecs;
    for (int j = -radius; j <= static_cast<int>(radius); ++j)
        for (int i = -radius; i <= static_cast<int>(radius); ++i)
            if (abs(i) + abs(j) <= static_cast<int>(radius))
                vecs.push_back(Vec2(i,j) + *this);
    return vecs;
}

std::vector<Vec2> Vec2::square_area(unsigned radius) const
{
    std::vector<Vec2> vecs;
    for (int j = -radius; j <= static_cast<int>(radius); ++j)
        for (int i = -radius; i <= static_cast<int>(radius); ++i)
            vecs.push_back(Vec2(i,j) + *this);
    return vecs;
}

std::ostream& operator<<(std::ostream& stream, const Vec2& vec)
{
    return stream << vec.x << " " << vec.y;
}

std::string Vec2::to_string() const
{
    std::ostringstream stream;
    stream << *this;
    return stream.str();
}
