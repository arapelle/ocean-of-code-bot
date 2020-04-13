#pragma once

#include "vec2.hpp"
#include <vector>
#include <ostream>
#include <iomanip>

using Position = Vec2;
using Offset = Vec2;

template <class Type>
class Grid
{
    using Ref = typename std::vector<Type>::reference;
    using Const_ref = typename std::vector<Type>::const_reference;

public:
    explicit Grid(int width = 0, int height = 0, const Type& value = Type()) { resize(width, height, value); }

    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline Const_ref get(int x, int y) const { return data_.at(y).at(x); }
    inline Ref get(int x, int y) { return data_.at(y).at(x); }
    inline Const_ref get(const Position& pos) const { return get(pos.x, pos.y); }
    inline Ref get(const Position& pos) { return get(pos.x, pos.y); }

    void clear()
    {
        width_ = 0;
        height_ = 0;
        data_.clear();
    }

    void resize(int width, int height, const Type& value = Type())
    {
        width_ = width;
        height_ = height;
        data_.resize(height_);
        for (auto& row : data_)
            row.resize(width_, value);
    }

    bool contains(const Position& pos) const
    {
        return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
    }

    std::size_t count(const Type& value) const
    {
        std::size_t res = 0;
        for (const auto& row : data_)
            for (const auto& element : row)
                if (element == value)
                    ++res;
        return res;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Grid<Type>& grid)
    {
        stream << "[GRID:" << grid.width() << " x " << grid.height() << "\n";
        for (const auto& row : grid.data_)
        {
            for (const auto& element : row)
                stream << std::setw(2) << element << " ";
            stream << std::endl;
        }
        stream << "]";
        return stream;
    }

protected:
    int width_ = -1;
    int height_ = -1;
    std::vector<std::vector<Type>> data_;
};
