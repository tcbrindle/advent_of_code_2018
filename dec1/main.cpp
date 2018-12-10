
#include "../common.hpp"

namespace {

int part_one(const std::vector<int>& vec)
{
    return std::accumulate(vec.cbegin(), vec.cend(), 0);
}

int part_two(const std::vector<int>& vec)
{
    std::set<int> set{};
    int total = 0;

    while (true) {
        for (int i : vec) {
            total += i;
            if (auto [iter, inserted] = set.insert(total); !inserted) {
                return total;
            }
        }
    }
}

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Give me a file to read\n";
        return -1;
    }

    using iter_t = nano::istream_iterator<int>;
    std::ifstream file(argv[1]);
    const std::vector<int> vec(iter_t{file}, iter_t{});

    fmt::print("Part 1 result is {}\n", part_one(vec));
    fmt::print("Part 2 result is {}\n", part_two(vec));
}