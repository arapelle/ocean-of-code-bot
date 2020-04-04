#ifdef NDEBUG
#undef NDEBUG
#endif

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <cstdlib>
#include <cassert>

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
        default: std::cerr << "ERR: Bad Move" << std::endl;
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

class Map
{
public:
    Map(int width = 0, int height = 0) { resize(width, height); }

    void fill_from_stream(std::istream& stream)
    {
        std::string line;
        for (auto& row : grid_)
        {
            std::getline(stream, line);
            for (int i = 0; i < width_; ++i)
                row[i] = Square(line.at(i));
        }
    }

    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline const Square& get(int x, int y) const { return grid_.at(y).at(x); }
    inline Square& get(int x, int y) { return grid_.at(y).at(x); }
    inline const Square& get(const Position& pos) const { return get(pos.x, pos.y); }
    inline Square& get(const Position& pos) { return get(pos.x, pos.y); }

    void clear() { width_ = 0; height_ = 0; grid_.clear(); }

    void clear_visit(int actor_id)
    {
        for (auto& row : grid_)
            for (auto& square : row)
                square.unset_visited(actor_id);
    }

    void resize(int width, int height)
    {
        width_ = width;
        height_ = height;
        grid_.resize(height_);
        for (auto& row : grid_)
            row.resize(width_);
    }

    bool contains(const Position& pos) const
    {
        return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
    }

    std::size_t accessibility(const Position& pos, int actor_id) const
    {
        std::size_t res = 0;
        const Square& square = get(pos);
        if (square.is_ocean() && !square.is_visited(actor_id))
        {
            std::cerr << "** acc.pos: " << pos << std::endl;
            for (unsigned dir = 0; dir < number_of_directions(); ++dir)
            {
                Position npos = pos.neighbour(Direction(dir));
                if (contains(npos))
                {
                    std::cerr << "** acc.npos: " << npos << std::endl;
                    const Square& nsquare = get(npos);
                    std::cerr << "** acc.npos.is_visited: " << nsquare.is_visited(actor_id) << std::endl;
                    std::cerr << "** acc.npos.is_ocean: " << nsquare.is_ocean() << std::endl;
                    if (!nsquare.is_visited(actor_id) && nsquare.is_ocean())
                        ++res;
                }
            }
        }
        return res;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Map& map)
    {
        for (const auto& row : map.grid_)
        {
            for (const auto& square : row)
                stream << square.type();
            stream << std::endl;
        }
        return stream;
    }

private:
    int width_;
    int height_;
    std::vector<std::vector<Square>> grid_;
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
               << "Opponent's Orders: " << info.opponentOrders << "\n";
        return stream;
    }

    friend std::istream& operator>>(std::istream& stream, Turn_info& info)
    {
        stream >> info.x >> info.y >> info.myLife >> info.oppLife >> info.torpedoCooldown
               >> info.sonarCooldown >> info.silenceCooldown >> info.mineCooldown;
        stream.ignore();
        stream >> info.sonarResult;
        stream.ignore();
        std::getline(std::cin, info.opponentOrders);
        return stream;
    }
};

struct Tool
{
    int cooldown = -1;

    bool is_available() const { return cooldown >= 0; }
    bool is_ready() const { return cooldown == 0; }
};

class Player_info
{
public:
    inline static constexpr std::size_t max_history_size = 5;

    struct Status
    {
        Position position;
        int hp = -1;
    };

    int id = -1;
    Status status;
    std::deque<Status> history_status;

    const Position& position() const { return status.position; }
    Position& position() { return status.position; }
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
};

class Opponent : public Player_info
{
public:

private:
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

    Toolkit toolkit;
    const Tool& torpedo() const { return toolkit.torpedo; }
    Tool& torpedo() { return toolkit.torpedo; }
    const Tool& sonar() const { return toolkit.sonar; }
    Tool& sonar() { return toolkit.sonar; }
    const Tool& silence() const { return toolkit.silence; }
    Tool& silence() { return toolkit.silence; }
    const Tool& mine() const { return toolkit.mine; }
    Tool& mine() { return toolkit.mine; }

private:
};

class Game
{
public:
    Game(std::istream& istream, std::ostream& ostream, std::ostream& dstream)
         : istrm_(istream), ostrm_(ostream), dstrm_(dstream)
    {}

    // START
    void init()
    {
        istrm_ >> game_info_ >> avatar_;
        istrm_.ignore();
        map_.resize(game_info_.map_width, game_info_.map_height);
        map_.fill_from_stream(istrm_);

        opponent_.id = avatar_.id == 0 ? 1 : 0;
    }

    void print_start_info() const
    {
        dstrm_ << game_info_.map_width << " " << game_info_.map_height << "  " << avatar_.id << std::endl;
        dstrm_ << map_ << std::endl;
    }

    Position choose_start_position()
    {
        for (int j = 0; j < map_.height(); ++j)
            for (int i = map_.width()-1; i > 0; ++i)
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
        dstrm_ << turn_info << std::endl;
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
            dstrm_ << "** npos: " << npos << std::endl;
            if (map_.contains(npos))
            {
                std::size_t acc = map_.accessibility(npos, avatar_.id);
                dstrm_ << "** acc: " << acc << std::endl;
                tdirs[acc].push_back(dir);
            }
        }
        unsigned max_acc = number_of_directions();
        for (; max_acc > 0; --max_acc)
            if (!tdirs[max_acc].empty())
                break;
        if (max_acc > 0)
            return tdirs[max_acc].front();
        dstrm_ << "* max_acc: " << max_acc << std::endl;
        return Bad;
    }

    void do_actions()
    {
        Direction move_dir = move_direction();
        dstrm_ << "* move_dir: " << dir_to_string(move_dir) << std::endl;
        if (dir_is_valid(move_dir))
            ostrm_ << move_action(move_dir) << " TORPEDO" << std::endl;
        else
        {
            ostrm_ << "SURFACE TORPEDO" << std::endl;
            map_.clear_visit(avatar_.id);
        }
    }

    // actions formatting:
    std::string move_action(Direction dir)
    {
        return std::string("MOVE ") + dir_to_string(dir);
    }

    // play turn:
    void play_turn()
    {
        Turn_info turn_info;
        istrm_ >> turn_info;
        update_data(turn_info);
        do_actions();
    }

    // MISC
    const Game_info& game_info() const { return game_info_; }
    const Map& map() const { return map_; }
    const Player_info& avatar() const { return avatar_; }
    const Player_info& opponent() const { return opponent_; }

private:
    Game_info game_info_;
    Map map_;
    Avatar avatar_;
    Opponent opponent_;

    std::istream& istrm_;
    std::ostream& ostrm_;
    std::ostream& dstrm_;
};

int main()
{
    Game game(std::cin, std::cout, std::cerr);
    game.init();

    game.play_start_actions();
    while (true)
        game.play_turn();

    return EXIT_SUCCESS;
}
