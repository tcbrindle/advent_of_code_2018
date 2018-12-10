
#include "../common.hpp"

#include <list>

namespace {

template <typename Iter, typename Cont>
Iter next_circ(Iter it, Cont& cont, int dist = 1)
{
    if (dist > 0) {
        while (dist-- > 0) {
            if (++it == cont.end()) {
                it = cont.begin();
            }
        }
    } else if (dist < 0) {
        while (dist++ < 0) {
            if (it == cont.begin()) {
                it = cont.end();
            }
            --it;
        }
    }
    return it;
}

int64_t calculate_score(int num_players, int num_marbles)
{
    std::vector<int64_t> scores(num_players);
    std::list<int> marbles{0};
    auto cur = marbles.begin();
    int current_player = 0;
    auto advance_player = [&] { if (++current_player == num_players) current_player = 0; };

    for (int i = 1; i <= num_marbles; i++)  {
        if (i % 23 == 0) {
            const auto next_pos = next_circ(cur, marbles, -7);
            scores[current_player] += *next_pos + i;
            cur = marbles.erase(next_pos);
        } else {
            auto next_pos = next_circ(cur, marbles);
            // We need to insert *after* this position, but std::list::insert
            // inserts *before* the given iterator, so adjust it by one
            cur = marbles.insert(++next_pos, i);
        }
        advance_player();
    }

    return nano::max(scores);
}

constexpr struct {
    int num_players;
    int num_marbles;
    int expected_result;
} test_data[] = {
    {9, 25, 32},
    {10, 1618, 8317},
    {13, 7999, 146373},
    {17, 1104, 2764},
    {21, 6111, 54718},
    {30, 5807, 37305}
};

}

int main()
{
#if 1
    for (const auto& t : test_data) {
        const auto res = calculate_score(t.num_players, t.num_marbles);
        if (res != t.expected_result) {
            fmt::print(stderr, "ERROR: with {} players and {} marbles, expected {}, got {}\n",
                       t.num_players, t.num_marbles, t.expected_result, res);
        }
    }
#endif

    fmt::print("For {} players and {} marbles, highest score was {}\n",
               412, 71646, calculate_score(412, 71646));
    fmt::print("For {} players and {} marbles, highest score was {}\n",
               412, 71464 * 100, calculate_score(412, 71646 * 100));
}