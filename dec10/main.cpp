
#include "../common.hpp"

namespace {

struct point {
    int64_t x = 0;
    int64_t y = 0;
};

struct velocity {
    int vx = 0;
    int vy = 0;
};

constexpr point operator+(const point& p, const velocity& v)
{
    return {p.x + v.vx, p.y + v.vy};
}

constexpr point operator-(const point& p, const velocity& v)
{
    return {p.x - v.vx, p.y - v.vy};
}

struct bounds {
    int64_t x_min, x_max, y_min, y_max;

    int64_t width() const { return  x_max - x_min; }
    int64_t height() const { return y_max - y_min; }
    int64_t size() const { return width() * height(); }
};

bounds calculate_bounds(const std::vector<point> pts)
{
    assert(!pts.empty());
    bounds b{pts[0].x, pts[0].x, pts[0].y, pts[0].y};

    for (auto it = nano::next(pts.begin()); it != pts.end(); ++it) {
        b.x_min = nano::min(b.x_min, it->x);
        b.x_max = nano::max(b.x_max, it->x);
        b.y_min = nano::min(b.y_min, it->y);
        b.y_max = nano::max(b.y_max, it->y);
    }

    return b;
}

void print_points(const std::vector<point>& pts)
{
    const auto b = calculate_bounds(pts);
    std::vector<std::string> out(b.height() + 1, std::string(b.width() + 1, ' '));

    for (const point& p : pts) {
        out[p.y - b.y_min][p.x - b.x_min] = '*';
    }

    nano::copy(out, nano::ostream_iterator<std::string>{std::cout, "\n"});
}

std::pair<std::vector<point>, std::vector<velocity>> read_input(std::istream& is)
{
    std::vector<point> pvec;
    std::vector<velocity> vvec;
    std::string s;

    while(std::getline(is, s)) {
        point p{};
        velocity v{};
        std::sscanf(s.c_str(), "position=<%lld, %lld> velocity=<%d, %d>", &p.x, &p.y, &v.vx, &v.vy);
        pvec.push_back(std::move(p));
        vvec.push_back(std::move(v));
    }

    return {std::move(pvec), std::move(vvec)};
}

constexpr auto& test_data = R"(position=< 9,  1> velocity=< 0,  2>
position=< 7,  0> velocity=<-1,  0>
position=< 3, -2> velocity=<-1,  1>
position=< 6, 10> velocity=<-2, -1>
position=< 2, -4> velocity=< 2,  2>
position=<-6, 10> velocity=< 2, -2>
position=< 1,  8> velocity=< 1, -1>
position=< 1,  7> velocity=< 1,  0>
position=<-3, 11> velocity=< 1, -2>
position=< 7,  6> velocity=<-1, -1>
position=<-2,  3> velocity=< 1,  0>
position=<-4,  3> velocity=< 2,  0>
position=<10, -3> velocity=<-1,  1>
position=< 5, 11> velocity=< 1, -2>
position=< 4,  7> velocity=< 0, -1>
position=< 8, -2> velocity=< 0,  1>
position=<15,  0> velocity=<-2,  0>
position=< 1,  6> velocity=< 1,  0>
position=< 8,  9> velocity=< 0, -1>
position=< 3,  3> velocity=<-1,  1>
position=< 0,  5> velocity=< 0, -1>
position=<-2,  2> velocity=< 2,  0>
position=< 5, -2> velocity=< 1,  2>
position=< 1,  4> velocity=< 2,  1>
position=<-2,  7> velocity=< 2, -2>
position=< 3,  6> velocity=<-1, -1>
position=< 5,  0> velocity=< 1,  0>
position=<-6,  0> velocity=< 2,  0>
position=< 5,  9> velocity=< 1, -2>
position=<14,  7> velocity=<-2,  0>
position=<-3,  6> velocity=< 2, -1>)";

}

int main(int argc, char** argv)
{
#if 1
    if (argc < 2) {
        fmt::print(stderr, "Need data\n");
        return -1;
    }

    std::fstream file(argv[1]);
    auto [points, velocities] = read_input(file);
#else
    std::istringstream iss(test_data);
    auto [points, velocities] = read_input(iss);
#endif

    auto size = calculate_bounds(points).size();
    int iter_counter = 0;

    while (true) {
        // "Process" one second
        nano::transform(points, velocities, points.begin(), std::plus<>{});
        const auto new_size = calculate_bounds(points).size();

        if  (new_size > size) {
            // "Untransform" the vector
            nano::transform(points, velocities, points.begin(), std::minus<>{});
            print_points(points);
            fmt::print("Found a solution after {} seconds\n", iter_counter);
            break;
        }

        size = new_size;
        ++iter_counter;
    }
}