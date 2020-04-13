#include "game.hpp"
#include "random.hpp"
#include "log.hpp"
#include <algorithm>
#include <map>

void Game::init()
{
    istrm_ >> game_info_ >> avatar_;
    istrm_.ignore();
    map_.resize(game_info_.map_width, game_info_.map_height);
    map_.fill_from_stream(istrm_);
    map_.set_sector_size(default_sector_width(), default_sector_height());

    opponent_.id = avatar_.id == 0 ? 1 : 0;
    opponent_.init();
}

void Game::print_start_info() const
{
    info() << game_info_.map_width << " " << game_info_.map_height << "  " << avatar_.id << std::endl;
    info() << map_ << std::endl;
}

Position Game::choose_start_position()
{
    std::map<std::size_t, std::vector<Position>, std::greater<std::size_t>> mpos;
    for (int j = 0; j < map_.height(); ++j)
        for (int i = 0; i < map_.width(); ++i)
        {
            Position pos(i,j);
            if (map_.get(pos).is_ocean())
            {
                std::size_t zone_size = map_.number_of_reachable_squares(pos, avatar_.id);
                mpos[zone_size].push_back(pos);
            }
        }
    auto& vpos = mpos.begin()->second;
    std::shuffle(vpos.begin(), vpos.end(), priv::rand_int_engine());
    auto iter = std::min_element(vpos.begin(), vpos.end(),
                                 [&](const Position& lhs, const Position& rhs)
    {
            return map_.accessibility(lhs, avatar_.id) < map_.accessibility(rhs, avatar_.id);
    });
    return *iter;
}

void Game::play_start_actions()
{
    Position start_position = choose_start_position();
    map_.get(start_position).set_visited(avatar_.id);
    ostrm_ << start_position << std::endl;
    print_start_info();
}

void Game::update_data(const Turn_info& turn_info)
{
    trace();
    info() << turn_info << std::endl;
    // Save player info
    avatar_.save_status();
    opponent_.save_status();
    // Update simple data
    //-- Avatar
    avatar_.position() = Position(turn_info.x, turn_info.y);
    map_.get(avatar_.position()).set_visited(avatar_.id);
    avatar_.hp() = turn_info.myLife;
    avatar_.torpedo().set_cooldown(turn_info.torpedoCooldown);
    avatar_.sonar().set_cooldown(turn_info.sonarCooldown);
    avatar_.silence().set_cooldown(turn_info.silenceCooldown);
    avatar_.mine().set_cooldown(turn_info.mineCooldown);
    avatar_.sonar().set_cooldown(turn_info.sonarCooldown);
    //-- Opponent
    opponent_.status.hp = turn_info.oppLife;
    // Update complex data
    avatar_.sonar().manage_info(turn_info.sonarResult);
    opponent_.update_data_with_orders(turn_info.opponentOrders);
    // Opponent status
    if (opponent_.sector_is_known())
        info() << "Opponent's sector: " << opponent_.sector << std::endl;
    if (opponent_.position_is_known())
        info() << "Opponent's pos: " << opponent_.position() << std::endl;
    info() << "Opponent's path: " << opponent_.relative_path.size() << std::endl;
}

Direction Game::move_direction()
{
    Position pos = avatar_.position();
    std::map<unsigned, std::vector<Direction>, std::greater<unsigned>> mdirs;
    for (unsigned i = 0; i < number_of_directions(); ++i)
    {
        Direction dir = Direction(i);
        Position npos = pos.neighbour(dir);
        if (map_.contains(npos))
        {
            unsigned zone_size = map_.number_of_reachable_squares(npos, avatar_.id);
            if (zone_size > 0)
                mdirs[zone_size].push_back(dir);
        }
    }
    if (mdirs.size() > 0)
    {
        const auto& dirs = mdirs.begin()->second;
//            return dirs.front();
        auto iter = std::min_element(dirs.begin(), dirs.end(),
                         [&](Direction ldir, Direction rdir)
                         {
                             return map_.accessibility(pos.neighbour(ldir), avatar_.id)
                                     < map_.accessibility(pos.neighbour(rdir), avatar_.id);
                         });
        return *iter;
    }
    return Bad;
}

void Game::do_actions()
{
    trace();
    do_main_actions();
    ostrm_ << " | MSG Turn " << turn_number_ << std::endl;
}

void Game::do_main_actions()
{
    if (opponent_.silence_used && avatar_.sonar().is_ready())
    {
        int sector = opponent_.most_marked_sector();
        avatar_.sonar().set_request(sector);
        ostrm_ << "SONAR " << sector << " | ";
    }

    Direction move_dir = move_direction();
    if (avatar_.silence().is_ready() && (turn_number() % 24 == 0))
    {
        unsigned distance = randint<unsigned>(0, 1);
        if (!dir_is_valid(move_dir))
        {
            move_dir = North;
            distance = 0;
        }
        ostrm_ << silence_action(move_dir, distance);
    }
    else if (dir_is_valid(move_dir))
    {
        info() << "ACTION: move_dir: " << dir_to_string(move_dir) << std::endl;
        ostrm_ << move_action(move_dir) << " " << load_submarine_tool();
    }
    else
    {
        info() << "ACTION: SURFACE" << std::endl;
        ostrm_ << "SURFACE";
        map_.clear_visit(avatar_.id);
    }
}

std::string_view Game::load_submarine_tool()
{
    if (avatar_.silence().is_available() && !avatar_.silence().is_ready())
        return "SILENCE";
    else if (avatar_.sonar().is_available() && !avatar_.sonar().is_ready())
        return "SONAR";
    else if (avatar_.torpedo().is_available() && !avatar_.torpedo().is_ready())
        return "TORPEDO";
    else if (avatar_.mine().is_available() && !avatar_.mine().is_ready())
        return "MINE";
    return "";
}

std::string Game::silence_action(Direction dir, unsigned distance)
{
    return std::string("SILENCE ") + dir_to_string(dir) + " " + std::to_string(distance);
}

std::string Game::move_action(Direction dir)
{
    return std::string("MOVE ") + dir_to_string(dir);
}

void Game::play_turn()
{
    Turn_info turn_info;
    istrm_ >> turn_info;

    info() << "---------------------------------------------" << std::endl;
    info() << "TURN NUMBER: " << turn_number_ << std::endl << std::flush;

    update_data(turn_info);
    do_actions();

    ++turn_number_;
}
