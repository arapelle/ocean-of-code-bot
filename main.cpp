#ifdef NDEBUG
//#undef NDEBUG
#endif

#include <iostream>
#include <random>
#include <iomanip>
#include <string>
#include <sstream>
#include <string_view>
#include <map>
#include <vector>
#include <deque>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <cassert>

#include "game.hpp"
#include "avatar.hpp"
#include "opponent.hpp"
#include "player.hpp"
#include "tool.hpp"
#include "turn_info.hpp"
#include "game_info.hpp"
#include "map.hpp"
#include "square.hpp"
#include "grid_with_sectors.hpp"
#include "grid.hpp"
#include "vec2.hpp"
#include "direction.hpp"
#include "log.hpp"
#include "random.hpp"

int main()
{
    Game game(std::cin, std::cout);
    game.init();

    game.play_start_actions();
    while (true)
        game.play_turn();

    return EXIT_SUCCESS;
}
