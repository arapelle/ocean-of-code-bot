#pragma once

#include "grid.hpp"
#include "log.hpp"

using Sector_position = Vec2;

template <class Type>
class Grid_with_sectors : public Grid<Type>
{
public:
    Grid_with_sectors(int width = 0, int height = 0)
        : Grid<Type>(width, height)
    {}

    inline int sector_width() const { return sector_width_; }
    inline int sector_height() const { return sector_height_; }

    void set_sector_size(int s_width, int s_height)
    {
        sector_width_ = s_width;
        sector_height_ = s_height;
    }

    int number_of_sectors_on_x() const { return this->width() / sector_width_; }
    int number_of_sectors_on_y() const { return this->height() / sector_height_; }
    int number_of_sectors() const { return number_of_sectors_on_x() * number_of_sectors_on_y(); }

    int position_to_sector_index(const Position& pos) const
    {
        if (this->contains(pos))
        {
            int sx = pos.x / sector_width();
            int sy = pos.y / sector_height();
            return sy * number_of_sectors_on_y() + sx + 1;
        }
        error() << "invalid position: " << pos << std::endl;
        return -1;
    }

    Sector_position sector_index_to_sector_position(int sector) const
    {
        if (sector < 1 || sector > number_of_sectors())
        {
            error() << "invalid sector: " << sector << std::endl;
            return Sector_position(-1,-1);
        }
        --sector;
        int y = sector / number_of_sectors_on_x();
        int x = sector % number_of_sectors_on_x();
        return Sector_position(x,y);
    }

    Position sector_origin(int sector) const
    {
        if (sector < 1 || sector > number_of_sectors())
        {
            error() << "invalid sector: " << sector << std::endl;
            return Position(-1,-1);
        }
        Sector_position spos = sector_index_to_sector_position(sector);
        return Position(spos.x * sector_width_, spos.y * sector_height_);
    }

    void fill_sector_if(int sector, const Type& value, const Type& value_to_replace)
    {
        Position origin = sector_origin(sector);
        for (int j = 0; j < sector_height_; ++j)
        {
            for (int i = 0; i < sector_width_; ++i)
            {
                Position pos = Position(i,j) + origin;
                Type& datum = this->get(pos);
                if (datum == value_to_replace)
                    datum = value;
            }
        }
    }

    std::size_t count_in_sector(int sector, const Type& value) const
    {
        std::size_t res = 0;

        Position origin = sector_origin(sector);
        for (int j = 0; j < sector_height_; ++j)
        {
            for (int i = 0; i < sector_width_; ++i)
            {
                Position pos = Position(i,j) + origin;
                const Type& datum = this->get(pos);
                if (datum == value)
                    ++res;
            }
        }

        return res;
    }

protected:
    int sector_width_ = -1;
    int sector_height_ = -1;
};
