
#include "../common.hpp"

#include <charconv>

namespace {

// Now I know what you're thinking. This version absolutely,
// positively *cannot* be faster than the commented-out version
// below. I mean look at it: it' got a loop with a conditional
// branch, whereas the other is just a couple of modulo operations
// and an addition. The generated code looks much worse too:
// see https://godbolt.org/z/joQhbT.
// And yet, the first version is measurably faster on x86_64 with
// both Clang and GCC at -O3. Go ahead, try it for yourself.
// No, I don't understand it either.
#ifndef SHORT_ADVANCE
void advance(size_t& cur, size_t dist, const size_t max)
{
    while (dist-- > 0) {
        if (++cur == max) {
            cur = 0;
        }
    }
}
#else
void advance(size_t& cur, size_t dist, const size_t max)
{
    cur += (dist % max);
    cur %= max;
}
#endif

auto part_one(size_t target_iters)
{
    std::vector<uint8_t> scores{3, 7};
    size_t pos1 = 0;
    size_t pos2 = 1;

    while (scores.size() < target_iters + 10) {
        uint8_t new_score = scores[pos1] + scores[pos2];

        if (new_score >= 10) {
            scores.push_back(new_score/10);
            scores.push_back(new_score % 10);
        } else {
            scores.push_back(new_score);
        }

        advance(pos1, 1 + scores[pos1], scores.size());
        advance(pos2, 1 + scores[pos2], scores.size());
    }

    std::string out(10, ' ');
    nano::transform(scores.begin() + target_iters, scores.begin() + target_iters + 10,
                    out.begin(), [](uint8_t i) { return i + '0'; });
    return out;
}

size_t part_two(std::string_view target_str)
{
    const std::vector<uint8_t> target_vec = [&]{
        std::vector<uint8_t> v(target_str.size());
        nano::transform(target_str, v.begin(), [](char c) -> uint8_t {
            return c - '0';
        });
        return v;
    }();
    const auto target_size = target_vec.size();

    std::vector<uint8_t> scores{3, 7};
    size_t pos1 = 0;
    size_t pos2 = 1;

    while (true) {
        uint8_t new_score = scores[pos1] + scores[pos2];

        if (new_score >= 10) {
            scores.push_back(new_score/10);
            scores.push_back(new_score % 10);

            const auto sub = nano::search(nano::subrange(scores.cend() - target_size - 1, scores.cend()), target_vec);
            if (!sub.empty()) {
                return nano::distance(scores.begin(), sub.begin());
            }
        } else {
            scores.push_back(new_score);

            if (nano::equal(nano::subrange(scores.cend() - target_size, scores.cend()), target_vec)) {
                return scores.size() - target_size;
            }
        }

        advance(pos1, 1 + scores[pos1], scores.size());
        advance(pos2, 1 + scores[pos2], scores.size());
    }
}

std::optional<size_t> to_size_t(const char* const str)
{
    const auto last = str + std::strlen(str);
    size_t out = 0;
    const auto res = std::from_chars(str, last, out);
    if (res.ptr != last) {
        return std::nullopt;
    }
    return out;
}

}

int main(int argc, char** argv)
{
    assert(part_one(9) == "5158916779");
    assert(part_one(5) == "0124515891");
    assert(part_one(18) == "9251071085");
    assert(part_one(2018) == "5941429882");

    assert(part_two("51589") == 9);
    assert(part_two("01245") == 5);
    assert(part_two("92510") == 18);
    assert(part_two("59414") == 2018);

    if (argc < 2) {
        fmt::print(stderr, "No input\n");
        return 1;
    }

    const auto target = to_size_t(argv[1]);
    if (!target) {
        fmt::print(stderr, "Argument was not a number\n");
        return 2;
    }

    fmt::print("Part one: after {} recipes, the next ten were {}\n", *target, part_one(*target));
    fmt::print("Part two: before the sequence {}, the number of recipes was {}\n", argv[1], part_two(argv[1]));
}