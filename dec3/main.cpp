
#include "../common.hpp"

namespace {

struct claim {
    int id = 0;
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    static claim parse(const std::string& str)
    {
        claim c;
        int width, height;
        std::sscanf(str.c_str(), "#%d @ %d,%d: %dx%d", &c.id, &c.left, &c.top, &width, &height);
        c.right = c.left + width;
        c.bottom = c.top + height;

        return c;
    }
};

bool overlap(const claim& x, const claim& y)
{
    return y.right > x.left &&
           y.bottom > x.top &&
           x.right > y.left &&
           x.bottom > y.top;
}

std::vector<claim> read_claims(const char* input_file)
{
    std::ifstream file(input_file);

    std::vector<claim> claims{};
    std::string str;

    while (std::getline(file, str)) {
        claims.push_back(claim::parse(str));
    }

    return claims;
}

std::pair<int, int> get_max_values(const std::vector<claim>& claims)
{
    return {nano::max(claims, nano::less<>{}, &claim::right).right,
            nano::max(claims, nano::less<>{}, &claim::bottom).bottom};
}

enum class claim_status : int8_t { none, single, multiple };

void inc_status(claim_status& s)
{
    s = (s == claim_status::none ? claim_status::single : claim_status::multiple);
}

}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "give me an input file";
        return 1;
    }

    const auto claims = read_claims(argv[1]);

    // Part one
    {
        // Calculate the max values of claims that we have been given
        // this always seems to be [1000, 1000] or thereabouts, but
        // it's not explicitly stated in the problem description
        const auto [width, height] = get_max_values(claims);
        std::vector<claim_status> fabric(width * height, claim_status::none);

        for (const auto& cl : claims) {
            for (int i = cl.left; i < cl.right; ++i) {
                for (int j = cl.top; j < cl.bottom; ++j) {
                    inc_status(fabric[i * height + j]);
                }
            }
        }

        fmt::print("{} squares of fabric are within two or more claims\n",
                  nano::count(fabric, claim_status::multiple));
    }

    // Part two
    {
        const auto no_overlap = [&](const auto& x) {
            return nano::find_if(claims, [x](const auto& y) {
                return x.id != y.id && overlap(x, y);
            }) == claims.end();
        };

        if (const auto iter = nano::find_if(claims, no_overlap); iter != claims.end()) {
            fmt::print("Claim #{} does not overlap with any others\n", iter->id);
        } else {
            fmt::print("There were no non-overlapping claims\n");
        }
    }
}
