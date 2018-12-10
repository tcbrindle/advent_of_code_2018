
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

namespace {

template <typename Iter>
int read_node_metadata(Iter& iter, Iter last)
{
    int num_children = *iter++;
    int num_meta = *iter++;
    int total = 0;

    for (int i = 0; i < num_children; i++) {
        total += read_node_metadata(iter, last);
    }

    for (int i = 0; i < num_meta; i++) {
        total += *iter++;
    }

    return total;
}

template <typename Iter>
int sum_metadata(Iter first, Iter last)
{
    // Here's a sneaky thing -- we don't actually have to construct the tree (yet)
    int meta_total = 0;

    while (first != last) {
        meta_total += read_node_metadata(first, last);
    }

    return meta_total;
}

constexpr auto& test_data = "2 3 0 3 10 11 12 1 1 0 1 99 2 1 1 2";

}

int main(int argc, char** argv)
{
#if 1
    if (argc < 2) {
        std::cerr << "Feed me data!\n";
        return -1;
    }

    std::ifstream is(argv[1]);
#else
    std::istringstream is(test_data);
#endif

    std::cout << "Got " << sum_metadata(std::istream_iterator<int>(is), std::istream_iterator<int>{}) << std::endl;
}


