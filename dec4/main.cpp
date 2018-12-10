
#include "../common.hpp"

#include "../extern/date.h"

#include <charconv>
#include <variant>

namespace {

using namespace std::chrono_literals;

using timestamp = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;

enum class guard_id : int {};

int to_int(guard_id i) { return static_cast<int>(i); }

struct start_shift_event {
    timestamp time{};
    guard_id id{};
};

struct sleep_event {
    timestamp time{};
};

struct wake_event {
    timestamp time{};
};

using event = std::variant<start_shift_event, sleep_event, wake_event>;

timestamp get_timestamp(const event& ev)
{
    return std::visit([](const auto& e) { return e.time; }, ev);
}

guard_id get_guard_id(const event& ev)
{
    assert(std::holds_alternative<start_shift_event>(ev));
    return std::get<start_shift_event>(ev).id;
}

timestamp parse_time(const std::string_view str)
{
    // There *must* be an easier way to do this...
    int year, month, day, hour, minute;
    std::sscanf(str.data(), "[%d-%d-%d %d:%d]", &year, &month, &day, &hour, &minute);
    const auto d =  date::year{year}/date::month(month)/date::day(day);
    return date::sys_days{d} + std::chrono::hours{hour} + std::chrono::minutes{minute};
}

// All this to avoid using std::regex...
guard_id parse_guard_id(const std::string_view str)
{
    const auto start = nano::next(nano::find(str, '#'));
    const auto end = nano::find(start, str.end(), ' ');

    int i = 0;
    std::from_chars(&*start, &*end, i);
    return guard_id{i};
}

event parse_event(const std::string_view str)
{
    const auto time = parse_time(str);

    const auto event_str = str.substr(19);

    switch (event_str[0]) {
    case 'f':
        return sleep_event{time};
    case 'w':
        return wake_event{time};
    case 'G':
        return start_shift_event{time, parse_guard_id(event_str)};
    default:
        assert(false);
    }
}

using event_log = std::vector<event>;

event_log build_event_log(std::istream& is)
{
    event_log e;
    std::string s;

    while (std::getline(is, s)) {
        e.push_back(parse_event(s));
    }

    return e;
}

const std::string test_event_log = R"([1518-11-01 00:00] Guard #10 begins shift
[1518-11-01 00:05] falls asleep
[1518-11-01 00:25] wakes up
[1518-11-01 00:30] falls asleep
[1518-11-01 00:55] wakes up
[1518-11-01 23:58] Guard #99 begins shift
[1518-11-02 00:40] falls asleep
[1518-11-02 00:50] wakes up
[1518-11-03 00:05] Guard #10 begins shift
[1518-11-03 00:24] falls asleep
[1518-11-03 00:29] wakes up
[1518-11-04 00:02] Guard #99 begins shift
[1518-11-04 00:36] falls asleep
[1518-11-04 00:46] wakes up
[1518-11-05 00:03] Guard #99 begins shift
[1518-11-05 00:45] falls asleep
[1518-11-05 00:55] wakes up
)";


struct sleep_record {
    void add_sleep(const timestamp start, const timestamp end)
    {
        const auto midnight = date::floor<date::days>(start);
        for (auto m = start; m < end; m += 1min) {
            ++arr[(m - midnight).count()];
        }
    }

    void merge(const sleep_record& other)
    {
        nano::transform(arr, other.arr, arr.begin(), std::plus<int>{});
    }

    std::chrono::minutes get_total() const
    {
        return std::chrono::minutes{std::accumulate(arr.begin(), arr.end(), 0)};
    }

    auto begin() const { return arr.begin(); }
    auto end() const { return arr.end(); }

private:
    std::array<int, 60> arr{};
};

using sleep_log = std::map<guard_id, sleep_record>;

template <typename Iter>
sleep_record calculate_sleep_record(Iter& iter, const Iter last)
{
    sleep_record rec{};

    while (iter != last) {
        if (std::holds_alternative<start_shift_event>(*iter)) {
            break;
        }

        // We must be at a sleep event
        assert(std::holds_alternative<sleep_event>(*iter));
        const auto sleep_start = get_timestamp(*iter);

        ++iter;
        assert(std::holds_alternative<wake_event>(*iter));
        const auto sleep_end = get_timestamp(*iter);

        rec.add_sleep(sleep_start, sleep_end);

        ++iter;
    }

    return rec;
}

sleep_log build_sleep_log(const event_log& elog)
{
    sleep_log slog{};

    auto iter = elog.begin();
    const auto last = elog.end();

    while (iter != last) {
        // We must be at a "start shift" event
        assert(std::holds_alternative<start_shift_event>(*iter));
        const auto id = get_guard_id(*iter);

        ++iter; // We are now at either a sleep event or a start_shift event
        if (std::holds_alternative<start_shift_event>(*iter)) {
            continue;
        }

        slog[id].merge(calculate_sleep_record(iter, last));
    }

    return slog;
}

}

int main(int argc, char** argv)
{
#if 1
    if (argc < 2) {
        fmt::print(stderr, "Provide me with some input, sir!\n");
        return -1;
    }

    std::ifstream is(argv[1]);
#else
    std::istringstream is{test_event_log};
#endif

    const auto elog = [&] {
        auto e = build_event_log(is);
        nano::sort(e, nano::less<>{}, get_timestamp);
        return e;
    }();

    const auto slog = build_sleep_log(elog);

    // Part One
    {
        const auto& [id, record] = *nano::max_element(slog, nano::less<>{}, [](const auto& p) {
            return p.second.get_total();
        });

        const auto sleepiest_minute = nano::distance(record.begin(), nano::max_element(record));

        fmt::print("Sleepiest guard was #{} ({} minutes)\n", to_int(id), record.get_total().count());
        fmt::print("Sleepiest minute was {}\n", sleepiest_minute);
        fmt::print("Product: {}\n", to_int(id) * sleepiest_minute);
    }

    // Part Two
    {
        const auto get_sleepiest_minute = [](const auto& record) {
            return nano::max_element(record);
        };

        const auto& [id, record] = *nano::max_element(slog, nano::less<>{}, [&](const auto& el) {
            return *get_sleepiest_minute(el.second);
        });

        fmt::print("Guard #{} was most frequently asleep on the same minute\n", to_int(id));
        const auto minute = nano::distance(record.begin(), get_sleepiest_minute(record));
        fmt::print("That was minute {}\n", minute);
        fmt::print("Product: {}\n", to_int(id) * minute);
    }
}


