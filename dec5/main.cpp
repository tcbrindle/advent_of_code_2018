
#include "../common.hpp"

namespace {

constexpr bool is_upper(char c) { return c >= 'A' && c <= 'Z'; }

constexpr char to_lower(char c) { return is_upper(c) ? c - ('A' - 'a') : c; }

struct letter_compare {
    static constexpr bool case_compare_char(char a, char b)
    {
        return is_upper(a) && !is_upper(b) && b == to_lower(a);
    }

    constexpr bool operator()(char a, char b) const
    {
        return case_compare_char(a, b) || case_compare_char(b, a);
    }
};

void process_str(std::string& str)
{
    auto iter = str.begin();

    while (iter != str.end()) {
        const auto new_iter = nano::adjacent_find(iter, str.end(), letter_compare{});
        if (new_iter == str.end()) {
            break;
        }
        iter = str.erase(new_iter, nano::next(new_iter, 2));
    }
}

std::string fully_process(std::string str)
{
    auto old_len = str.size();
    while (true) {
        process_str(str);
        if (str.size() == old_len) {
            return str;
        }
        old_len = str.size();
    }
}

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Give me an input file\n";
        return -1;
    }

    const std::string original = [&] {
        std::ifstream in(argv[1]);
        std::string s;
        in >> s;
        return s;
    }();

    //const std::string original = "dabAcCaCBAcCcaDA";
    const auto fully_processed = fully_process(original);
    fmt::print("Part 1: fully processed length: {}\n", fully_processed.size());

    {
        std::array<int, 26> results{};

        for (int i = 0; i < 26; i++) {
            const char remove_c = 'a' + i;

            std::string str = fully_processed;
            str.erase(nano::remove(str, remove_c, to_lower), str.end());

            results[i] = fully_process(std::move(str)).size();
        }

        const auto iter = nano::min_element(results);

        fmt::print("Shortest length was {}, found by removing element {}\n",
                   *iter, (char)('a' + nano::distance(results.begin(), iter)));
    }
}