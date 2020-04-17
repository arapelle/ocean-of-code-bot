#include "avatar.hpp"

bool Avatar::has_lost_life() const
{
    return previous_status().hp - hp() > 0;
}
