#pragma once

#include <cstdint>

struct Square
{
    Square() {}
    explicit Square(char type) : value_(type) {}
    char type() const { return value_; }
    void set_type(char type) { value_ = type; }
    bool is_ocean() const { return value_ == '.'; }
    bool is_visited(int actor_id) const { return visited_mask_ & (1<<actor_id); }
    void set_visited(int actor_id) { visited_mask_ |= (1<<actor_id); }
    void unset_visited(int actor_id) { visited_mask_ &= ~(1<<actor_id); }

private:
    char value_ = '?';
    uint64_t visited_mask_ = 0;
};
