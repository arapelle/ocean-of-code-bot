#ifdef NDEBUG
#undef NDEBUG
#endif

#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <cassert>

namespace priv
{
std::mt19937_64& rand_int_engine()
{
    static std::mt19937_64 rnd_engine(std::random_device{}());
    return rnd_engine;
}
}

template <class NT>
NT randint (NT a, NT b)
{
    using Dist = std::uniform_int_distribution<NT>;
    static thread_local Dist dist;
    return dist(priv::rand_int_engine(), typename Dist::param_type{a, b});
}

std::ostream& info() { return std::cerr; }
std::ostream& error() { return std::cerr << "ERROR: "; }
std::ostream& debug() { return std::cerr << "DEBUG: "; }

struct Game_info
{
    int map_width = -1;
    int map_height = -1;

    friend std::istream& operator>>(std::istream& stream, Game_info& info)
    {
        return stream >> info.map_width >> info.map_height;
    }
};

enum Direction : char
{
    North,
    East,
    South,
    West,
    Undefined = '?',
    Bad = '%',
};


std::size_t number_of_directions() { return 4; }

bool dir_is_valid(Direction dir) { return unsigned(dir) < number_of_directions(); }

Direction char_to_dir(char ch)
{
    switch (ch)
    {
    case 'N': return North;
    case 'E': return East;
    case 'S': return South;
    case 'W': return West;
    default: error() << "Bad char dir" << std::endl;
        ;
    }
    return Bad;
}

std::string dir_to_string(Direction dir)
{
    switch (dir)
    {
    case North: return "N";
    case East: return "E";
    case South: return "S";
    case West: return "W";
    case Undefined: return "?";
    case Bad: return "%";
    }
    return "ERR: @_@";
}

class Vec2
{
public:
    int x = 0;
    int y = 0;

    Vec2() = default;
    Vec2(int x, int y)
        : x(x), y(y)
    {}

    Vec2& operator+=(const Vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    friend Vec2 operator+(const Vec2& lfs, const Vec2& rhs)
    {
        Vec2 res = lfs;
        res += rhs;
        return res;
    }

    Vec2& operator-=(const Vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    friend Vec2 operator-(const Vec2& lfs, const Vec2& rhs)
    {
        Vec2 res = lfs;
        res -= rhs;
        return res;
    }

    Vec2& move(Direction dir)
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

    Vec2 neighbour(Direction dir) const { Vec2 vec(*this); vec.move(dir); return vec; }

    friend std::ostream& operator<<(std::ostream& stream, const Vec2& vec)
    {
        return stream << vec.x << " " << vec.y;
    }
};

using Position = Vec2;
using Sector_position = Vec2;

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

template <class Type>
class Grid
{
public:
    explicit Grid(int width = 0, int height = 0) { resize(width, height); }

    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline const Type& get(int x, int y) const { return data_.at(y).at(x); }
    inline Type& get(int x, int y) { return data_.at(y).at(x); }
    inline const Type& get(const Position& pos) const { return get(pos.x, pos.y); }
    inline Type& get(const Position& pos) { return get(pos.x, pos.y); }

    void clear() { width_ = 0; height_ = 0; data_.clear(); }

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

    friend std::ostream& operator<<(std::ostream& stream, const Grid<Type>& grid)
    {
        stream << "[GRID:" << grid.width() << " x " << grid.height() << "\n";
        for (const auto& row : grid.data_)
        {
            for (const auto& element : row)
                stream << element << " ";
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

class Map : public Grid<Square>
{
public:
    Map(int width = 0, int height = 0)
        : Grid<Square>(width, height)
    {}

    inline int sector_width() const { return sector_width_; }
    inline int sector_height() const { return sector_height_; }

    void set_sector_size(int s_width, int s_height)
    {
        sector_width_ = s_width;
        sector_height_ = s_height;
    }

    int number_of_sectors_on_x() const { return width() / sector_width_; }
    int number_of_sectors_on_y() const { return height() / sector_height_; }
    int number_of_sectors() const { return number_of_sectors_on_x() * number_of_sectors_on_y(); }

    int position_to_sector_index(const Position& pos) const
    {
        if (contains(pos))
        {
            int sx = pos.x / sector_width();
            int sy = pos.y / sector_height();
            return sy * sector_height() + sx + 1;
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
        int y = sector / width();
        int x = sector % width();
        return Sector_position(x,y);
    }

    Position sector_origin(int sector) const
    {
        if (sector < 1 || sector > number_of_sectors())
        {
            error() << "invlaid sector: " << sector << std::endl;
            return Position(-1,-1);
        }
        Sector_position spos = sector_index_to_sector_position(sector);
        return Position(spos.x * sector_width_, spos.y * sector_height_);
    }

    void fill_from_stream(std::istream& stream)
    {
        std::string line;
        for (auto& row : data_)
        {
            std::getline(stream, line);
            for (int i = 0; i < width_; ++i)
                row[i] = Square(line.at(i));
        }
    }

    void clear_visit(int actor_id)
    {
        for (auto& row : data_)
            for (auto& square : row)
                square.unset_visited(actor_id);
    }

    std::size_t accessibility(const Position& pos, int actor_id) const
    {
        std::size_t res = 0;
        const Square& square = get(pos);
        if (square.is_ocean() && !square.is_visited(actor_id))
        {
            for (unsigned dir = 0; dir < number_of_directions(); ++dir)
            {
                Position npos = pos.neighbour(Direction(dir));
                if (contains(npos))
                {
                    const Square& nsquare = get(npos);
                    if (!nsquare.is_visited(actor_id) && nsquare.is_ocean())
                        ++res;
                }
            }
        }
        return res;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Map& map)
    {
        for (const auto& row : map.data_)
        {
            for (const auto& square : row)
                stream << square.type();
            stream << std::endl;
        }
        return stream;
    }

protected:
    int sector_width_ = -1;
    int sector_height_ = -1;
};

struct Turn_info
{
    // Self info
    int x = -1;
    int y = -1;
    int myLife = -1;
    int oppLife = -1;
    int torpedoCooldown = -1;
    int sonarCooldown = -1;
    int silenceCooldown = -1;
    int mineCooldown = -1;
    // Sonar info
    std::string sonarResult = "?";
    // Opponent info
    std::string opponentOrders = "?";

    friend std::ostream& operator<<(std::ostream& stream, const Turn_info& info)
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

    friend std::istream& operator>>(std::istream& stream, Turn_info& info)
    {
        stream >> info.x >> info.y >> info.myLife >> info.oppLife >> info.torpedoCooldown
               >> info.sonarCooldown >> info.silenceCooldown >> info.mineCooldown;
        stream.ignore();
        stream >> info.sonarResult;
        stream.ignore();
        std::getline(stream, info.opponentOrders);
        return stream;
    }
};

struct Tool
{
    int cooldown = -1;

    bool is_available() const { return cooldown >= 0; }
    bool is_ready() const { return cooldown == 0; }
};

class Game;

class Player_info
{
public:
    inline static constexpr std::size_t max_history_size = 5;

    struct Status
    {
        Position position = Position(-1, -1);
        int hp = -1;
    };

    Player_info()
    {}

    explicit Player_info(Game& game)
        : game(&game)
    {}

    const Position& position() const { return status.position; }
    Position& position() { return status.position; }
    bool position_is_known() const { return position().x >= 0 && position().y >= 0; }
    const int& hp() const { return status.hp; }
    int& hp() { return status.hp; }

    const Status& previous_status() const { return history_status.back(); }
    void save_status()
    {
        while (history_status.size() >= max_history_size)
            history_status.pop_front();
        history_status.push_back(status);
    }

    friend std::istream& operator>>(std::istream& stream, Player_info& info)
    {
        return stream >> info.id;
    }

public:
    Game* game = nullptr;
    int id = -1;
    Status status;
    std::deque<Status> history_status;
};

class Opponent : public Player_info
{
public:
    using Mark_map = Grid<int16_t>;

    explicit Opponent(Game& game)
        : Player_info(game)
    {}

    void init();

    inline bool sector_is_known() const { return sector >= 0; }
    void reset_sector() { sector = -1; }

    void treat_order(const std::string_view& order)
    {
        std::istringstream iss(std::string(order.begin(), order.end()));
        std::string command;
        iss >> command;
        if (command == "SURFACE")
        {
            iss >> sector;
            relative_path.clear();
            update_pos_info_with_sector_();
        }
        else if (command == "MOVE")
        {
            char dir_ch;
            iss >> dir_ch;
            Direction dir = char_to_dir(dir_ch);
            relative_path.push_back(dir);
            update_pos_info_with_move_dir_(dir);
        }
        else if (command == "SILENCE")
        {
            //TODO
        }
    }

    void update_data(const std::string& orders)
    {
        std::size_t index = 0;
        std::size_t end_index = 0;
        for (; index < orders.length(); index = end_index + 1)
        {
            end_index = orders.find('|', index);
            end_index = std::min(end_index, orders.length());
            std::string_view order(&orders[index], end_index);
            if (!order.empty())
                treat_order(order);
        }
    }

    const Mark_map& mark_map() const { return mark_map_; }

private:
    void update_pos_info_with_sector_();
    void update_pos_info_with_move_dir_(Direction dir);

public:
    int sector = -1;
    std::vector<Direction> relative_path;
    Mark_map mark_map_;
};

class Avatar : public Player_info
{
public:
    struct Toolkit
    {
        Tool torpedo;
        Tool sonar;
        Tool silence;
        Tool mine;
    };

public:
    explicit Avatar(Game& game)
        : Player_info(game)
    {}

    Toolkit toolkit;
    const Tool& torpedo() const { return toolkit.torpedo; }
    Tool& torpedo() { return toolkit.torpedo; }
    const Tool& sonar() const { return toolkit.sonar; }
    Tool& sonar() { return toolkit.sonar; }
    const Tool& silence() const { return toolkit.silence; }
    Tool& silence() { return toolkit.silence; }
    const Tool& mine() const { return toolkit.mine; }
    Tool& mine() { return toolkit.mine; }
};

class Game
{
public:
    static int default_sector_width() { return 5; }
    static int default_sector_height() { return 5; }

    Game(std::istream& istream, std::ostream& ostream)
         : avatar_(*this), opponent_(*this),
           istrm_(istream), ostrm_(ostream)
    {}

    // START
    void init()
    {
        istrm_ >> game_info_ >> avatar_;
        istrm_.ignore();
        map_.resize(game_info_.map_width, game_info_.map_height);
        map_.fill_from_stream(istrm_);
        map_.set_sector_size(default_sector_width(), default_sector_height());

        opponent_.id = avatar_.id == 0 ? 1 : 0;
        opponent_.init();
    }

    void print_start_info() const
    {
        info() << game_info_.map_width << " " << game_info_.map_height << "  " << avatar_.id << std::endl;
        info() << map_ << std::endl;
    }

    Position choose_start_position()
    {
        for (int j = 0; j < map_.height(); ++j)
            for (int i = map_.width()-1; i > 0; --i)
                if (map_.get(i,j).is_ocean())
                    return Position(i,j);
        return Position(7,7);
    }

    void play_start_actions()
    {
        Position start_position = choose_start_position();
        map_.get(start_position).set_visited(avatar_.id);
        ostrm_ << start_position << std::endl;
        print_start_info();
    }

    // TURN
    // update:
    void update_data(const Turn_info& turn_info)
    {
        info() << turn_info << std::endl;
        avatar_.save_status();
        avatar_.position() = Position(turn_info.x, turn_info.y);
        map_.get(avatar_.position()).set_visited(avatar_.id);
        avatar_.hp() = turn_info.myLife;
        avatar_.torpedo().cooldown = turn_info.torpedoCooldown;
        avatar_.sonar().cooldown = turn_info.sonarCooldown;
        avatar_.silence().cooldown = turn_info.silenceCooldown;
        avatar_.mine().cooldown = turn_info.mineCooldown;
        opponent_.save_status();
        opponent_.status.hp = turn_info.oppLife;
        opponent_.update_data(turn_info.opponentOrders);
        if (opponent_.sector_is_known())
            info() << "Opponent's sector: " << opponent_.sector << std::endl;
        if (opponent_.position_is_known())
            info() << "Opponent's pos: " << opponent_.position() << std::endl;
        info() << "Opponent's path: " << opponent_.relative_path.size() << std::endl;
    }

    // ia:
    Direction move_direction()
    {
        Position pos = avatar_.position();
        std::vector<std::vector<Direction>> tdirs(number_of_directions() + 1);
        for (unsigned i = 0; i < number_of_directions(); ++i)
        {
            Direction dir = Direction(i);
            Position npos = pos.neighbour(dir);
            if (map_.contains(npos))
            {
                std::size_t acc = map_.accessibility(npos, avatar_.id);
                tdirs[acc].push_back(dir);
            }
        }
        unsigned max_acc = number_of_directions();
        for (; max_acc > 0; --max_acc)
            if (!tdirs[max_acc].empty())
                break;
        if (max_acc > 0)
            return tdirs[max_acc].front();
        return Bad;
    }

    void do_actions()
    {
        Direction move_dir = move_direction();
        if (avatar_.silence().is_ready())
        {
            unsigned distance = randint<unsigned>(0, 1);
            if (!dir_is_valid(move_dir))
            {
                move_dir = North;
                distance = 0;
            }
            ostrm_ << silence_action(move_dir, distance) << std::endl;
        }
        else if (dir_is_valid(move_dir))
        {
            info() << "ACTION: move_dir: " << dir_to_string(move_dir) << std::endl;
            ostrm_ << move_action(move_dir) << " SILENCE" << std::endl;
        }
        else
        {
            info() << "ACTION: SURFACE" << std::endl;
            ostrm_ << "SURFACE SILENCE" << std::endl;
            map_.clear_visit(avatar_.id);
        }
    }

    // actions formatting:
    std::string silence_action(Direction dir, unsigned distance)
    {
        return std::string("SILENCE ") + dir_to_string(dir) + " " + std::to_string(distance);
    }

    std::string move_action(Direction dir)
    {
        return std::string("MOVE ") + dir_to_string(dir);
    }

    // play turn:
    void play_turn()
    {
        Turn_info turn_info;
        istrm_ >> turn_info;

        info() << "---------------------------------------------" << std::endl;
        info() << "TURN NUMBER: " << turn_number_ << std::endl << std::flush;

        update_data(turn_info);
        do_actions();

        ++turn_number_;
    }

    // MISC
    const Game_info& game_info() const { return game_info_; }
    const Map& map() const { return map_; }
    const Player_info& avatar() const { return avatar_; }
    const Player_info& opponent() const { return opponent_; }
    int turn_number() const { return turn_number_; }

private:
    int turn_number_ = 0;
    Game_info game_info_;
    Map map_;
    Avatar avatar_;
    Opponent opponent_;

    std::istream& istrm_;
    std::ostream& ostrm_;
};

void Opponent::init()
{
    const Map& map = game->map();

    mark_map_.resize(map.width(), map.height(), 0);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            if (!map.get(i,j).is_ocean())
                mark_map_.get(i,j) = -2;
        }
    }
}

void Opponent::update_pos_info_with_sector_()
{
    const Map& map = game->map();
    int previous_turn_number = game->turn_number() - 1;

    Position origin = map.sector_origin(sector);
    unsigned mark_count = 0;
    Position last_pos;
    for (int j = 0; j < map.sector_width(); ++j)
    {
        for (int i = 0; i < map.sector_width(); ++i)
        {
            Position pos = Position(i,j) + origin;
            if (mark_map_.get(pos) == previous_turn_number)
            {
                mark_map_.get(pos) = game->turn_number();
                ++mark_count;
                last_pos = pos;
            }
        }
    }
    if (mark_count == 1)
        position() = last_pos;
}

void Opponent::update_pos_info_with_move_dir_(Direction dir)
{
    const Map& map = game->map();
    int previous_turn_number = game->turn_number() - 1;

    unsigned mark_count = 0;
    Position last_pos;
    Mark_map next_mark_map(mark_map_);
    for (int j = 0; j < mark_map_.height(); ++j)
    {
        for (int i = 0; i < mark_map_.width(); ++i)
        {
            Position pos(i,j);
            Position npos = pos.neighbour(dir);
            if (mark_map_.get(pos) == previous_turn_number && map.contains(npos) && map.get(npos).is_ocean())
            {
                next_mark_map.get(npos) = game->turn_number();
                ++mark_count;
                last_pos = npos;
            }
        }
    }
    mark_map_ = next_mark_map;
    //TODO if mark_count > 1 && all marked squares are in the same sector:
    //         sector = visited_sector;

    if (mark_count == 1)
    {
        position() = last_pos;
        sector = map.position_to_sector_index(last_pos);
    }
}

int main()
{
    Game game(std::cin, std::cout);
    game.init();

    game.play_start_actions();
    while (true)
        game.play_turn();

    return EXIT_SUCCESS;
}
