
#include "../common.hpp"

#include <bitset>

namespace {

using state_t = std::array<std::uint32_t, 4>;

namespace op {

constexpr auto addr = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] + reg[b];
};

constexpr auto addi = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] + b;
};

constexpr auto mulr = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] * reg[b];
};

constexpr auto muli = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] * b;
};

constexpr auto banr = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] & reg[b];
};

constexpr auto bani = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] & b;
};

constexpr auto borr = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] | reg[b];
};

constexpr auto bori = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] | b;
};

constexpr auto setr = [](state_t& reg, uint32_t a, uint32_t, uint32_t c) {
    reg[c] = reg[a];
};

constexpr auto seti = [](state_t& reg, uint32_t a, uint32_t, uint32_t c) {
    reg[c] = a;
};

constexpr auto gtir = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = a > reg[b] ? 1 : 0;
};

constexpr auto gtri = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] > b ? 1 : 0;
};

constexpr auto gtrr = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] > reg[b] ? 1 : 0;
};

constexpr auto eqir = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = a == reg[b] ? 1 : 0;
};

constexpr auto eqri = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] == b ? 1 : 0;
};

constexpr auto eqrr = [](state_t& reg, uint32_t a, uint32_t b, uint32_t c) {
    reg[c] = reg[a] == reg[b] ? 1 : 0;
};

} // namespace op

using op_t = void (*) (state_t&, uint32_t, uint32_t, uint32_t);

constexpr std::array<op_t, 16> operations = {
    op::addr, op::addi, op::mulr, op::muli, op::banr,
    op::bani, op::borr, op::bori, op::setr, op::seti,
    op::gtir, op::gtri, op::gtrr,
    op::eqir, op::eqri, op::eqrr
};

state_t parse_state(const std::string& str)
{
    constexpr auto& format_str = "[%u, %u, %u, %u]";
    const auto pos = str.find('[');
    state_t state;
    std::sscanf(str.c_str() + pos, format_str, &state[0], &state[1], &state[2], &state[3]);
    return state;
}

//void print_state(const state_t& s) {
//    fmt::print("[{}, {}, {}, {}]\n", s[0], s[1], s[2], s[3]);
//}

struct instruction {
    uint32_t opcode;
    uint32_t a, b, c;
};

instruction parse_instruction(const std::string& str)
{
    constexpr auto& format_str = "%u %u %u %u";
    instruction i;
    std::sscanf(str.c_str(), format_str, &i.opcode, &i.a, &i.b, &i.c);
    return i;
}

//void print_instruction(const instruction& i)
//{
//    fmt::print("{} {} {} {}\n", i.opcode, i.a, i.b, i.c);
//}

constexpr auto& test_data1 =
R"(Before: [3, 2, 1, 1]
9 2 1 2
After:  [3, 2, 2, 1])";

struct sample {
    state_t pre;
    instruction inst;
    state_t post;
};

using sample_stream = std::vector<sample>;

sample_stream parse_samples(std::istream& is)
{
    sample_stream stream;
    std::string s;

    while (true) {
        if (!std::getline(is, s)) break;
        const state_t pre_state = parse_state(s);

        if (!std::getline(is, s)) break;
        const instruction i = parse_instruction(s);

        if (!std::getline(is, s)) break;
        const state_t post_state = parse_state(s);

        stream.push_back(sample{pre_state, i, post_state});

        if (!std::getline(is, s)) break;
    }

    return stream;
}

int part_one(const sample_stream& ss)
{
    int match3_count = 0;

    for (const auto& s : ss) {
        int match_count = 0;
        for (const auto& func : operations) {
            state_t state = s.pre;
            func(state, s.inst.a, s.inst.b, s.inst.c);
            if (state == s.post) {
                ++match_count;
            }
            if (match_count >= 3) {
                ++match3_count;
                break;
            }
        }
    }

    return match3_count;
}

struct possbilities_matrix {
    possbilities_matrix()
    {
        nano::for_each(pos, [](auto& b) { b.set(); });
    }

    void eliminate(size_t opcode, size_t idx)
    {
        if (pos[opcode].count() == 1) {
            return;
        }

        pos[opcode].reset(idx);

        if (pos[opcode].count() == 1) {
           // fmt::print("Setting row {} position {} to 0\n", opcode, get_val(opcode));
            assign(opcode, get_val(opcode));
        }
    }

    void assign(size_t opcode, size_t idx)
    {
        for (size_t i = 0; i < 16; i++) {
            if (i != opcode) {
                eliminate(i, idx);
            }
        }
    }

    size_t get_val(size_t opcode)
    {
        for (int i = 0; i < 16; i++) {
            if (pos[opcode][i]) {
                return i;
            }
        }
        throw std::runtime_error("This went wrong");
    }

    void print() const
    {
        for (int i = 0; i < 16; i++) {
            fmt::print("{:2}: {}\n", i, pos[i].to_string());
        }
    }

private:
    std::array<std::bitset<16>, 16> pos;
};

using opcode_map = std::array<op_t, 16>;

opcode_map build_opcode_map(const sample_stream& ss)
{
    possbilities_matrix mat;

    for (const auto& samp : ss)
    {
        for (int i = 0; i < 16; i++) {
            auto state = samp.pre;
            operations[i](state, samp.inst.a, samp.inst.b, samp.inst.c);
            if (state != samp.post) {
                mat.eliminate(samp.inst.opcode, i);
            }
        }
    }

    opcode_map m{};
    for (size_t i = 0; i < 16; i++) {
        m[i] = operations[mat.get_val(i)];
    }
    return m;
}

using instruction_stream = std::vector<instruction>;

instruction_stream parse_instruction_stream(std::istream& is)
{
    instruction_stream out;
    std::string s;

    while (std::getline(is, s)) {
        out.push_back(parse_instruction(s));
    }

    return out;
}

state_t process_instruction_stream(const instruction_stream& is, const opcode_map& ops)
{
    state_t state{0, 0, 0, 0};
    for (const auto& inst : is) {
        ops[inst.opcode](state, inst.a, inst.b, inst.c);
    }
    return state;
}

}

int main(int argc, char** argv)
{
    if (argc < 3) {
        fmt::print(stderr, "Expect two arguments: a file with samples (part one), and a file with instructions (part two)");
        return 1;
    }

    std::fstream is1(argv[1]);
    const auto stream = parse_samples(is1);
    fmt::print("Part one: {} instructions match 3 or more opcodes\n", part_one(stream));

    const auto map = build_opcode_map(stream);
    std::fstream is2(argv[2]);
    const auto istream = parse_instruction_stream(is2);
    const auto final_state = process_instruction_stream(istream, map);
    fmt::print("Part two: final value of register 0 was {}\n", final_state[0]);
}