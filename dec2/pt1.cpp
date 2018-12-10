
#include "../common.hpp"

namespace {

struct repeat_info {
    bool has_two = false;
    bool has_three = false;
};

repeat_info count_freqs(const std::string& str)
{
    std::array<int, 26> freqs{};
    for (const char c : str) {
        ++freqs[c - 'a'];
    }

    repeat_count r{};
    for (const int i : freqs) {
        if (i == 2) {
            r.has_two = true;
        } else if (i == 3) {
            r.has_three = true;
        }
    }

    return r;
}

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Give me a file to read\n";
        return -1;
    }

    std::ifstream file(argv[1]);
    using iter_t = std::istream_iterator<std::string>;
    const auto input = std::vector(iter_t{file}, iter_t{});

    std::vector<repeat_info> counts(input.size());
    nano::transform(input, counts.begin(), count_freqs);

    const auto two_count = nano::count_if(counts, &repeat_info::has_two);
    const auto three_count = nano::count_if(counts, &repeat_info::has_three);

    fmt::print("Part 1 result is {}\n", two_count * three_count);
}