
#include "../common.hpp"

#include <variant>

namespace {

namespace detail {

template <typename... Funcs>
struct visitor : Funcs...
{
    template <typename... Args>
    constexpr visitor(Args&&... args)
        : Funcs(std::forward<Args>(args))...
    {}

    using Funcs::operator()...;
};

}

template <typename... Fs>
constexpr auto make_visitor(Fs&&... fs)
{
    return detail::visitor<Fs...>{std::forward<Fs>(fs)...};
}

struct position {
    int x = 0;
    int y = 0;

    friend constexpr bool operator==(position lhs, position rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
    //friend constexpr bool operator!=(position lhs, position rhs) { return !(lhs == rhs); }
};

namespace dir {
    struct north {};
    struct south {};
    struct east {};
    struct west {};
}

using direction = std::variant<dir::north, dir::east, dir::south, dir::west>;

constexpr char to_char(const direction& d)
{
    return std::visit(make_visitor(
        [](dir::north) { return '^'; },
        [](dir::east) { return '>'; },
        [](dir::south) { return 'v'; },
        [](dir::west) { return '<'; }
    ), d);
}

namespace track_type {
    struct ew {};
    struct ns {};
    struct curve_ne {};
    struct curve_se {};
    struct isect {};
    struct none {};
}

using track = std::variant<track_type::ew, track_type::ns, track_type::curve_ne,
                           track_type::curve_se, track_type::isect, track_type::none>;

constexpr char to_char(const track& t)
{
    return std::visit(make_visitor(
        [](track_type::ew) { return '-'; },
        [](track_type::ns) { return '|'; },
        [](track_type::curve_ne) { return '\\'; },
        [](track_type::curve_se) { return '/'; },
        [](track_type::isect) { return '+'; },
        [](track_type::none) { return ' '; }
    ), t);
}

using network_t = std::vector<std::vector<track>>;

namespace isect {
    struct left {};
    struct middle {};
    struct right {};
};

using isect_dir = std::variant<isect::left, isect::middle, isect::right>;

enum class cart_id : int {};

class cart {
public:
    cart(int x, int y, direction d)
        : pos_{x, y},
          dir_(std::move(d))
    {}

    const position& get_position() const { return pos_; }
    const direction& get_direction() const { return dir_; }
    cart_id get_id() const { return id_; }

    void update(const network_t& network)
    {
        const auto& t = network[pos_.y][pos_.x];
        std::visit(make_visitor(
            [this](dir::north, track_type::ns, auto) { move_north(); },
            [this](dir::north, track_type::curve_ne, auto) { move_west(); },
            [this](dir::north, track_type::curve_se, auto) { move_east(); },
            [this](dir::north, track_type::isect, isect::left) { move_west(); next_dir_ = isect::middle{}; },
            [this](dir::north, track_type::isect, isect::middle) { move_north(); next_dir_ = isect::right{}; },
            [this](dir::north, track_type::isect, isect::right) { move_east(); next_dir_ = isect::left{}; },

            [this](dir::east, track_type::ew, auto) { move_east(); },
            [this](dir::east, track_type::curve_ne, auto) { move_south(); },
            [this](dir::east, track_type::curve_se, auto) { move_north(); },
            [this](dir::east, track_type::isect, isect::left) { move_north(); next_dir_ = isect::middle{}; },
            [this](dir::east, track_type::isect, isect::middle) { move_east(); next_dir_ = isect::right{}; },
            [this](dir::east, track_type::isect, isect::right) { move_south(); next_dir_ = isect::left{}; },

            [this](dir::south, track_type::ns, auto) { move_south(); },
            [this](dir::south, track_type::curve_ne, auto) { move_east(); },
            [this](dir::south, track_type::curve_se, auto) { move_west(); },
            [this](dir::south, track_type::isect, isect::left) { move_east(); next_dir_ = isect::middle{}; },
            [this](dir::south, track_type::isect, isect::middle) { move_south(); next_dir_ = isect::right{}; },
            [this](dir::south, track_type::isect, isect::right) { move_west(); next_dir_ = isect::left{}; },

            [this](dir::west, track_type::ew, auto) { move_west(); },
            [this](dir::west, track_type::curve_ne, auto) { move_north(); },
            [this](dir::west, track_type::curve_se, auto) { move_south(); },
            [this](dir::west, track_type::isect, isect::left) { move_south(); next_dir_ = isect::middle{}; },
            [this](dir::west, track_type::isect, isect::middle) { move_west(); next_dir_ = isect::right{}; },
            [this](dir::west, track_type::isect, isect::right) { move_north(); next_dir_ = isect::left{}; },

            [](auto d, auto t, auto) {
                throw std::runtime_error(fmt::format("Unhandled direction/track combination ({}, {})",
                                         to_char(d), to_char(t)));
            }
        ), dir_, t, next_dir_);
    }

    bool active = true;

private:
    void move_north() { dir_ = dir::north{}; --pos_.y; }
    void move_east() { dir_ = dir::east{}; ++pos_.x; }
    void move_south() { dir_ = dir::south{}; ++pos_.y; }
    void move_west() { dir_ = dir::west{}; --pos_.x; }

    static inline int id_counter_ = 0;
    position pos_{};
    direction dir_{};
    isect_dir next_dir_ = isect::left{};
    cart_id id_ = cart_id{id_counter_++};
};

bool collision(const cart& a, const cart& b)
{
    return a.active && b.active &&
           a.get_id() != b.get_id() &&
           a.get_position() == b.get_position();
}

class state {
public:
    explicit state(std::istream& is);

    position process_till_collision()
    {
        while (true) {
            sort_carts();

            for (auto& c : carts_) {
                c.update(network_);

                if (nano::any_of(carts_, [&c](const auto& o) { return collision(c, o); })) {
                    return c.get_position();
                }
            }
        }
    }

    void process_tick()
    {
        sort_carts();

        for (auto& c : carts_) {
            if (!c.active) {
                continue;
            }

            c.update(network_);

            for (auto& other : carts_) {
                if (collision(c, other)) {
                    c.active = false;
                    other.active = false;
                }
            }
        }

        // Prune inactive carts
        carts_.erase(nano::remove(carts_, false, &cart::active), carts_.end());
    }

    std::string to_string() const
    {
        std::vector<std::string> strings;
        nano::transform(network_, nano::back_inserter(strings), [](const auto& row) {
            std::string str;
            nano::transform(row, nano::back_inserter(str), [](const auto& t) {
                return to_char(t);
            });
            return str;
        });

       for (const auto& c : carts_) {
            const auto& pos = c.get_position();
            strings[pos.y][pos.x] = to_char(c.get_direction());
        }

        std::string out;
        for (auto& s : strings) {
            out += std::move(s);
            out += '\n';
        }
        return out;
    }

    const std::vector<cart>& get_carts() const { return carts_; }

private:
    void sort_carts()
    {
        nano::sort(carts_, [](const auto& lhs, const auto& rhs) {
            return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
        }, &cart::get_position);
    }

    network_t network_;
    std::vector<cart> carts_;
};

inline state::state(std::istream& is)
{
    std::string s;

    for (int j = 0; std::getline(is, s); j++)
    {
        for (size_t i = 0; i < s.size(); i++) {
            char& c = s[i];
            switch (c){
            case '^': carts_.emplace_back(i, j, dir::north{});  c = '|'; break;
            case '>': carts_.emplace_back(i, j, dir::east{});  c = '-';  break;
            case 'v': carts_.emplace_back(i, j, dir::south{}); c = '|'; break;
            case '<': carts_.emplace_back(i, j, dir::west{}); c = '-';  break;
            }
        }

        std::vector<track> line;
        nano::transform(s, nano::back_inserter(line), [&] (const char c) -> track {
            switch (c) {
            case '-': return track_type::ew{};
            case '|': return track_type::ns{};
            case '/': return track_type::curve_se{};
            case '\\': return track_type::curve_ne{};
            case '+': return track_type::isect{};
            case ' ': return track_type::none{};
            default:
                throw std::runtime_error(fmt::format("Unknown track type '{}'", c));
            }
        });

        network_.push_back(std::move(line));
    }
}

constexpr auto& test_input =
R"(/->-\
|   |  /----\
| /-+--+-\  |
| | |  | v  |
\-+-/  \-+--/
  \------/  )";

constexpr auto& test_input2 =
R"(/>-<\
|   |
| /<+-\
| | | v
\>+</ |
  |   ^
  \<->/)";

}

int main(int argc, char** argv)
{
    // Tests
    {
        std::istringstream is{test_input};
        assert((state{is}.process_till_collision() == position{7, 3}));
    }

    {
        std::istringstream is{test_input2};
        state s{is};
        while(s.get_carts().size() > 1) {
            s.process_tick();
        }
        assert(!s.get_carts().empty());
        assert((s.get_carts().front().get_position() == position{6, 4}));
    }

    if (argc < 2) {
        fmt::print(stderr, "Please provide some input\n");
        return 1;
    }

    std::ifstream is{argv[1]};
    const state initial{is};

    // Part one
    {
        auto state = initial;
        const auto pos = state.process_till_collision();
        fmt::print("Part one: got collision at {},{}\n", pos.x, pos.y);
    }

    // Part two
    {
        auto state = initial;
        while (state.get_carts().size() > 1) {
            state.process_tick();
        }
        assert(!state.get_carts().empty());
        const auto pos = state.get_carts().front().get_position();
        fmt::print("Part two: last remaining cart has position {},{}\n", pos.x, pos.y);
    }
}