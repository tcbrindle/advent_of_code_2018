
#include "../common.hpp"

namespace {

struct node {
    std::vector<std::unique_ptr<node>> children;
    std::vector<int> metadata;

    int get_value() const
    {
        if (children.empty()) {
            return std::accumulate(metadata.begin(), metadata.end(), 0);
        }

        int total = 0;
        for (const int offbyone : metadata) {
            const int idx = offbyone - 1;
            if (idx >= 0 && idx < children.size()) {
                total += children[idx]->get_value();
            }
        }

        return total;
    }
};

template <typename Iter>
std::unique_ptr<node> make_node(Iter& iter)
{
    const int num_children = *iter++;
    const int num_metas = *iter++;

    auto ptr = std::make_unique<node>();

    for (int i = 0; i < num_children; i++) {
        ptr->children.push_back(make_node(iter));
    }

    iter = nano::copy_n(iter, num_metas, nano::back_inserter(ptr->metadata)).in;

    return ptr;
}

int get_total_values(std::istream& is)
{
    auto iter = nano::istream_iterator<int>(is);
    const auto head = make_node(iter);

    return head->get_value();
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

    auto iter = nano::istream_iterator<int>(is);
    const auto root = make_node(iter);

    fmt::print("Root node has value {}\n", root->get_value());
}


