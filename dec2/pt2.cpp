
#include "../common.hpp"

namespace {

std::string build_matching_string(const std::string& str1, const std::string& str2)
{
    std::string out;
    for (int i = 0; i < str1.size(); ++i) {
        if (str1[i] == str2[i]) {
            out += str1[i];
        }
    }
    return out;
}

template <typename Cont>
std::string compare_strings(const Cont& cont)
{
    auto first = nano::begin(cont);
    const auto last = nano::cend(cont);

    while (first != last) {
        auto next = first;

        while (++next != last) {
            const auto& str1 = *first;
            const auto& str2 = *next;

            if (str1.size() != str2.size()) {
                continue;
            }

            int num_diffs = 0;

            for (auto i = 0; i < str1.size(); ++i) {
                if (str1[i] != str2[i]) {
                    ++num_diffs;
                }
                if (num_diffs > 1) {
                    break;
                }
            }

            if (num_diffs == 1) {
                // Yay, we've found strings that differ by one!
                return build_matching_string(str1, str2);
            }
        }

        ++first;
    }

    return "<no matching strings found>";
}

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Give me a file to read\n";
        return -1;
    }

    std::ifstream file(argv[1]);
    using iter_t = nano::istream_iterator<std::string>;
    const std::vector input(iter_t(file), iter_t{});

    fmt::print("Part two result was {}\n", compare_strings(input));
}