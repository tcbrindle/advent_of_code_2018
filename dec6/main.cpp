
#include "../common.hpp"

namespace {

struct point {
    int x = 0;
    int y = 0;
};

// Calculates the manhattan distance between two points
int mh_distance(const point& p1, const point& p2)
{
    return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

// Given a range of points, finds the element which is closest
// to the test point p.
// If more than one element is equally close, returns an empty optional
// Otherwise, returns an optional containing an iterator into points
template <typename Points>
auto find_unique_nearest(const point& p, const Points& points)
    -> std::optional<nano::iterator_t<const Points>>
{
    // "Project" each point in points into its distance from p
    const auto proj = [&] (const point& t) { return mh_distance(p, t); };

    // Find an iterator to the point in points which is closest to p
    auto iter = nano::min_element(points, nano::less<>{}, proj);

    // The selected point may not be unique -- let's count how many points
    // are equally close
    const auto dist = mh_distance(p, *iter);
    const auto count = nano::count(points, dist, proj);

    if (count < 2) {
        return std::optional{std::move(iter)};
    } else {
        return std::nullopt;
    }
}

struct boundary {
    int min_x = 0;
    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
};

template <typename Points>
boundary calculate_boundary(const Points& pts)
{
    const auto [min_x, max_x] = nano::minmax(pts, nano::less<>{}, &point::x);
    const auto [min_y, max_y] = nano::minmax(pts, nano::less<>{}, &point::y);
    return {min_x.x, max_x.x, min_y.y, max_y.y};
}

std::vector<point> read_points(std::istream& is)
{
    std::vector<point> v;
    std::string s;
    while(std::getline(is, s)) {
        point p;
        std::sscanf(s.c_str(), "%d, %d", &p.x, &p.y);
        v.push_back(std::move(p));
    }
    return v;
}

const std::array<point, 6> test_coords = {
    point{ 1, 1 },
    { 1, 6 },
    { 8, 3 },
    { 3, 4 },
    { 5, 5 },
    { 8, 9 }
};

}

int main(int argc, char** argv)
{
#if 1
    if (argc < 2) {
        std::cerr << "I require some input\n";
        return -1;
    }

    std::fstream file(argv[1]);
    const auto points = read_points(file);

    constexpr int distance_limit = 10'000;

#else
    const auto& points = test_coords;
    constexpr int distance_limit = 32;
#endif

    // Works out the boundaries
    const auto b = calculate_boundary(points);

    // Part 1

    // For each point p inside the boundary, find its nearest point.
    auto nearest_area = std::vector<int>(nano::size(points));

    for (auto i = b.min_x; i <= b.max_x; ++i) {
        for (auto j = b.min_y; j <= b.max_y; ++j) {
            const auto opt = find_unique_nearest(point{i, j}, points);
            if (opt) {
                const auto idx = nano::distance(nano::begin(points), *opt);
                ++nearest_area[idx];
            }
        }
    }

    // We now need to discount all those points which have "infinite" area
    // Basically, a point has "infinite area" if one of the boundary points is
    // closest to it.
    // We'll go through and "fix" this by running over all found boundary edges
    // and setting nearsest_area[i] to zero
    for (auto i = b.min_x; i <= b.max_x; ++i) {
        const auto o1 = find_unique_nearest(point{i, b.min_y}, points);
        const auto o2 = find_unique_nearest(point{i, b.max_y}, points);
        if (o1) nearest_area[*o1 - points.begin()] = 0;
        if (o2) nearest_area[*o2 - points.begin()] = 0;
    }

    for (auto j = b.min_y; j <= b.max_y; j++) {
        const auto o1 = find_unique_nearest(point{b.min_x, j}, points);
        const auto o2 = find_unique_nearest(point{b.max_x, j}, points);
        if (o1) nearest_area[*o1 - points.begin()] = 0;
        if (o2) nearest_area[*o2 - points.begin()] = 0;
    }


    std::cout << "Largest was " << nano::max(nearest_area) << '\n';


    // Part 2
    {
        // For each internal point, calculate the distance to each given point, and then
        // sum those distances. Easy!
        int points_in_region = 0;

        for (auto i = b.min_x; i <= b.max_x; ++i) {
            for (auto j = b.min_y; j <= b.max_y; ++j) {
                const point test_point{i, j};
                const int total_dist = std::accumulate(points.begin(), points.end(), 0,
                                                       [&](int total, const point& p) {
                                                           return total + mh_distance(p, test_point);
                                                       });
                if (total_dist < distance_limit) {
                    ++points_in_region;
                }
            }
        }

        std::cout << "The size of the region is " << points_in_region << '\n';
    }
}