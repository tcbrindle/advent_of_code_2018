
#include "../common.hpp"

#include <deque>
#include <unordered_map>

namespace {

struct plants {
    explicit plants(std::istream& is)
    {
        std::string s;
        std::getline(is, s);

        for (char c : s.substr(15)) {
            deque_.push_back(c == '#');
        }

        std::getline(is, s); // ignore blank line

        while (std::getline(is, s)) {
            uint8_t idx = 0;
            for (int i = 0; i < 5; i++) {
                if (s[i] == '#') {
                    idx += (1 << i);
                }
            }
            mapping_[idx] = (s[9] == '#');
        }
    }

    std::string to_string() const
    {
        std::string out;
        nano::transform(deque_, nano::back_inserter(out), [](bool b) {
            return b ? '#' : '.';
        });
        return out;
    }

    int get_sum() const
    {
        int total = 0;
        for (size_t i = 0; i < deque_.size(); ++i) {
            if (deque_[i]) {
                total += i - offset_;
            }
        }

        return total;
    }

    void process()
    {
        deque_.insert(deque_.begin(), 4, false);
        deque_.insert(deque_.end(), 4, false);
        offset_ += 2;
        auto next = deque_;
        next.clear();

        for (size_t i = 0; i < deque_.size() - 4; i++) {
            next.push_back(mapping_.at(get_uint8(i)));
        }

        deque_ = std::move(next);
        trim();
    }

private:
    uint8_t get_uint8(size_t offset) const
    {
        uint8_t out = 0;
        for (int i = 0; i < 5; i++) {
            if (deque_[offset + i]) {
                out += (1 << i);
            }
        }
        return out;
    }

    void trim()
    {
        auto iter = nano::find(deque_, true);
        offset_ -= nano::distance(deque_.begin(), iter);
        deque_.erase(deque_.begin(), iter);

        iter = nano::find(deque_.rbegin(), deque_.rend(), true).base();
        deque_.erase(nano::next(iter), deque_.end());
    }

    std::deque<bool> deque_;
    std::array<bool, 32> mapping_{};
    int offset_ = 0;
};

int get_sum_after(plants p, int generations)
{
    while (generations-- > 0) {
        p.process();
    }
    return p.get_sum();
}

constexpr auto& test_data = R"(initial state: #..#.#..##......###...###

...## => #
..#.. => #
.#... => #
.#.#. => #
.#.## => #
.##.. => #
.#### => #
#.#.# => #
#.### => #
##.#. => #
##.## => #
###.. => #
###.# => #
####. => #)";

}

int main(int argc, char** argv)
{
#if 1
    std::fstream is(argv[1]);
#else
    std::istringstream is{test_data};
#endif

    const plants orig(is);

    // Part one: 20 generations
    fmt::print("After 20 generations, sum was {}\n", get_sum_after(orig, 20));

    // Part two: 50 billion generations
    {
        // Hypothesis: eventually we fall into a steady state, adding a constant number
        // of plants each generation
        const int diff = [&] {
            const int gen = 1000;
            return get_sum_after(orig, gen+1) - get_sum_after(orig, gen);
        }();

        fmt::print("Eventually we seem to be adding {} plants every generation\n", diff);

        // Okay, so now we need to iterate through until we reach the steady state
        const auto [steady_gens, steady_sum] = [&] {
            plants p = orig;
            int last_sum = p.get_sum();
            int iterations = 0;

            while (true) {
                p.process();
                ++iterations;
                const int sum = p.get_sum();
                if (sum - last_sum == diff) {
                    last_sum = sum;
                    break;
                }
                last_sum = sum;
            }
            return std::pair{iterations, last_sum};
        }();

        fmt::print("It seems we reached a steady state after {} generations, when the sum was {}\n", steady_gens, steady_sum);
        const auto target_gens = 50'000'000'000;
        const auto calculated = steady_sum + (target_gens - steady_gens) * diff;
        //const auto actual = get_sum_after(orig, target_gens);

        fmt::print("Calculated sum after {} generations was {}\n", target_gens, calculated);
    }
}
