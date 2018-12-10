
#include "../common.hpp"

#include <deque>

namespace {

using steps_map = std::map<char, std::string>;

steps_map parse_steps(std::istream& is)
{
    std::string s;
    steps_map out;

    while (std::getline(is, s)) {
        char prereq = s[5];
        char step = s[36];
        out[prereq] += ""; // HACKHACKHACK
        out[step] += prereq;
    }

    return out;
}

std::string part_one(steps_map map)
{
    std::string output{};

    while (!map.empty()) {
        // First, let's find all of the entries with no prerequisites
        std::string no_prereqs;
        for (const auto& [step, prereqs] : map) {
            if (prereqs.empty()) {
                no_prereqs += step;
            }
        }

        // Now, we find the first (alphabetically) entry on that list, and remove it
        assert(!no_prereqs.empty());
        char rem = nano::min(no_prereqs);
        // ...and add it to our output
        output += rem;

        // Now we remove that from the map...
        map.erase(rem);
        for (auto& [step, prereqs] : map) {
            prereqs.erase(nano::remove(prereqs, rem), prereqs.end());
        }
    }

    return output;
}

enum class task : char {};
enum class worker_id : int {};
using std::chrono::seconds;
using namespace std::chrono_literals;

struct worker_pool {
    worker_pool(const steps_map& smap, int num_workers, seconds time_offset = 0s)
        : workers(num_workers),
          map(smap),
          time_offset(time_offset)
    {
        enqueue_tasks();
        assign_workers();
    }

    void process_second()
    {
        bool any_finished = false;

        for (auto& w : workers) {
            if (!w.is_idle() && --w.time_remaining <= 0s) {
                auto t = w.current;
                w.current = worker::idle;
                any_finished = true;
                finished_task(t);
            }
        }

        if (any_finished) {
            enqueue_tasks();
            assign_workers();
        }

        ++seconds_counter;
    }

    bool done() const
    {
        return map.empty() && queue.empty() && nano::all_of(workers, &worker::is_idle);
    }

    seconds elapsed() const { return seconds_counter; }

private:
    void enqueue_tasks()
    {
        for (auto& [step, prereqs] : map) {
            if (prereqs.empty()) {
                queue.push_back(task{step});
                map.erase(step);
            }
        }
    }

    void finished_task(task t)
    {
        for (auto& [step, prereqs] : map) {
            char c = static_cast<char>(t);
            prereqs.erase(nano::remove(prereqs, c), prereqs.end());
        }
    }

    void assign_workers()
    {
        if (queue.empty()) {
            return;
        }

        // Otherwise, assign work
        while (!queue.empty()) {
            if (find_worker(queue.front())) {
                queue.pop_front();
            } else {
                return;
            }
        }
    }

    bool find_worker(task t)
    {
        // Assign to first idle worker, if any
        for (auto& w : workers) {
            if (w.is_idle()) {
                fmt::print("{}: assigning task {} to worker {}\n", seconds_counter.count(), (char) t, (int) w.id);
                w.current = t;
                w.time_remaining = time_offset + seconds{char(t) - 'A'} + 1s;
                return true;
            }
        }

        // No available workers
        return false;
    }

    struct worker {
        inline static int next_id = 0;
        static constexpr task idle{'.'};

        worker_id id{next_id++};
        task current = idle;
        seconds time_remaining{0};

        bool is_idle() const { return current == idle; }
    };

    std::vector<worker> workers;
    std::deque<task> queue;
    steps_map map;
    seconds time_offset{0};
    seconds seconds_counter{0};
};



seconds part_two(const steps_map& steps, int num_workers, seconds time_offset)
{
    worker_pool pool(steps, num_workers, time_offset);

    while (!pool.done()) {
        pool.process_second();
    }

    return pool.elapsed();
}


constexpr auto& test_instructions = R"(Step C must be finished before step A can begin.
Step C must be finished before step F can begin.
Step A must be finished before step B can begin.
Step A must be finished before step D can begin.
Step B must be finished before step E can begin.
Step D must be finished before step E can begin.
Step F must be finished before step E can begin.)";

}

int main(int argc, char** argv)
{
#if 1
    if (argc < 2) {
        std::cerr << "input please\n";
        return -1;
    }

    std::fstream is(argv[1]);

    constexpr int num_workers = 5;
    constexpr auto time_offset = 60s;
#else
    std::istringstream is(test_instructions);
    constexpr int num_workers = 2;
    constexpr auto time_offset = 0s;
#endif

    const auto map = parse_steps(is);

    std::cout << "Part one solution: " << part_one(map) << '\n';
    int pt2 = part_two(map, num_workers, time_offset).count();
    std::cout << "Part two took " << pt2 << " seconds\n";
}
