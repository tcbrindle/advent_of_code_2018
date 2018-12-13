
#include <array>
#include <cstdint>
#include <cstdio>
#include <tuple>

namespace {

constexpr int size = 300;

constexpr int8_t calculate_cell_power(int x, int y, int serial)
{
    const int rack_id = x + 10;
    int power_level = rack_id * y;
    power_level += serial;
    power_level *= rack_id;
    power_level /= 100;
    power_level %= 10;
    power_level -= 5;
    return power_level;
}

#ifdef TESTING
static_assert(calculate_cell_power(3, 5, 8) == 4);
static_assert(calculate_cell_power(122, 79, 57) == -5);
static_assert(calculate_cell_power(217, 196, 39) == 0);
static_assert(calculate_cell_power(101, 153, 71) == 4);
#endif

constexpr auto min = [](auto x, auto y) { return x < y ? x : y; };
constexpr auto to_idx = [](int x, int y) { return (x - 1) * size + y - 1; };

using grid = std::array<int8_t, size * size>;

constexpr grid calculate_grid(const int serial)
{
    grid out{};
    for (int i = 1; i <= size; i++) {
        for (int j = 1; j <= size; j++) {
            out[to_idx(i, j)] = calculate_cell_power(i, j, serial);
        }
    }
    return out;
}

constexpr int calculate_square_power(const int x, const int y, const grid& grid)
{
    int total = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            total += grid[to_idx(x + i, y + j)];
        }
    }
    return total;
}

#ifdef TESTING
constexpr auto grid18 = calculate_grid(18);
constexpr auto grid42 = calculate_grid(42);

static_assert(calculate_square_power(33, 45, grid18) == 29);
static_assert(calculate_square_power(21, 61, grid42) == 30);
#endif

using coord = std::tuple<int, int>;

constexpr coord part_one(const int serial)
{
    int max_i = 0;
    int max_j = 0;
    int max_pow = 0;
    const auto grid = calculate_grid(serial);

    for (int i = 1; i <= size - 3; i++) {
        for (int j = 1; j <= size - 3; j++) {
            const int p = calculate_square_power(i, j, grid);
            if (p > max_pow) {
                max_pow = p;
                max_i = i;
                max_j = j;
            }
        }
    }

    return {max_i, max_j};
}

#ifdef TESTING
static_assert(part_one(18) == coord{33, 45});
static_assert(part_one(42) == coord{21, 61});
#endif

constexpr std::tuple<int, int> find_largest_square(const int x, const int y, const grid& grid)
{
    int max_sz = 0;
    int max_tot = 0;
    int last_tot = 0;

    for (int sz = 1; sz < min(size-x+1, size-y+1); sz++) {
        for (int i = 0; i < sz; i++) {
            last_tot += grid[to_idx(x + i, y + sz - 1)];
            last_tot += grid[to_idx(x + sz - 1, y + i)];
        }
        last_tot -= grid[to_idx(x + sz - 1, y + sz - 1)];

        if (last_tot > max_tot) {
            max_sz = sz;
            max_tot = last_tot;
        }
    }

    return {max_sz, max_tot};
}

#ifdef TESTING
static_assert(find_largest_square(90, 269, grid18) == std::tuple{16, 113});
static_assert(find_largest_square(232, 251, grid42) == std::tuple{12, 119});
#endif

constexpr std::tuple<int, int, int>
part_two(const int serial)
{
    std::tuple<int, int, int> out{0, 0, 0};
    int max_power = 0;
    const auto grid = calculate_grid(serial);

    for (int i = 1; i < size; i++) {
        for (int j = 1; j < size; j++) {
            const auto [sz, pow] = find_largest_square(i, j, grid);
            if (pow > max_power) {
                std::get<0>(out) = i;
                std::get<1>(out) = j;
                std::get<2>(out) = sz;
                max_power = pow;
            }
        }
    }

    return out;
}

#ifdef TESTING
static_assert(part_two(18) == std::tuple{90, 269, 16});
static_assert(part_two(42) == std::tuple{232, 251, 12});
#endif

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::printf("Give me a serial number\n");
        return -1;
    }

    const int serial = std::atoi(argv[1]);
    {
        const auto [x, y] = part_one(serial);
        std::printf("Serial %d has max at (%d,%d)\n", serial, x, y);
    }

    {
        const auto [x, y, s] = part_two(serial);
        std::printf("Part two: %d,%d,%d\n", x, y, s);
    }
}