#ifndef MODULE_H
#define MODULE_H

#include "ast/nodes_forward.h"
#include "ir/ir_forward.h"
#include "module_forward.h"

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace bytecode {

class modul final {

  public:
    void build();
    void write();

    explicit modul(std::string filename);
    modul(const modul &) = delete;
    modul & operator=(const modul &) = delete;

    modul(modul &&) noexcept = default;
    modul & operator=(modul &&) noexcept = default;

    ~modul() noexcept = default;

    enum reg : uint8_t {
        zero = 0,
        v0,
        v1,
        a0,
        a1,
        a2,
        a3,
        a4,
        a5,
        temp,
        s0,
        s1,
        s2,
        s3,
        s4,
        s5,
        s6,
        s7,
        s8,
        s9,
        s10,
        s11,
        s12,
        s13,
        s14,
        s15,
        s16,
        s17,
        s18,
        s19,
        sp,
        lr,
    };

    static_assert(reg::lr == 31);

  private:
    std::string filename;

    enum class func_num : uint8_t {};

    enum class opcode : uint8_t {
        r_type = 0,
        lui = 1,
        ori = 5,
        jal = 20,
        jr = 21,
        syscall = 63,
    };

    struct r_type {
        reg rd, rs1, rs2;
        uint8_t shamt;
        func_num func;
    };
    struct i_type {
        reg rd;
        reg rs;
        uint16_t imm;
    };
    struct j_type {
        reg rd;
        uint32_t imm;
    };
    struct s_type {
        reg rd;
        reg rs1;
        reg rs2;
        reg rs3;
        uint8_t func;
    };

    using instruction_data = std::variant<r_type, i_type, j_type, s_type>;
    struct instruction {
        opcode op;
        instruction_data data;

        instruction(opcode op, instruction_data && data)
            : op{op}
            , data{std::move(data)} {}

        [[nodiscard]] operator uint32_t() const;
    };

    void add_instruction(opcode, instruction_data &&);
    reg alloc_reg();

    struct program_data {
        std::vector<uint32_t> segment_table;
        std::vector<uint32_t> segment_data;
        uint32_t exec_start;
    };
    program_data layout_segments(uint32_t start_segment_table);

    struct function_details {
        std::vector<instruction> instructions;
        std::map<ir::operand, reg> allocated_registers;
        uint32_t number;
    };

    std::map<std::string, function_details> functions;
    std::string current_function;

    [[nodiscard]] const function_details & cur_func() const;

    [[nodiscard]] std::set<reg> used_registers() const;

    static constexpr uint32_t vm_text_start = 0x5000;
    static constexpr uint32_t vm_data_start = 0x4000;
    static constexpr uint32_t sp_start = 0x3000'0000;
    std::vector<uint8_t> data_segment;

    uint32_t func_num = 0;
};

} // namespace bytecode

#endif
