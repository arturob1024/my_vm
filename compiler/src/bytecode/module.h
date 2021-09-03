#ifndef MODULE_H
#define MODULE_H

#include "ast/nodes_forward.h"
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

    void add_top_level_item(ast::top_level * top_lvl);

    [[nodiscard]] size_t top_level_item_count() const noexcept { return top_lvl_items.size(); }

    // Top level item compilation

    void register_global(std::string id, const std::optional<std::string> & type,
                         ast::expression & value, bool constant);

    void register_function(std::string id, const std::vector<ast::typed_id> & params,
                           const std::optional<std::string> & type, ast::statement & body);

    void register_struct(std::string id, const std::vector<ast::typed_id> & params);

    // Statment compilation

    void call_function(std::string id, std::vector<compiled_expr> args);

    // Expression compilation

    compiled_expr compile_literal(const std::string & value, ast::type typ);

    compiled_expr compile_binary_op(ast::binary_operation, compiled_expr, compiled_expr);

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
    explicit modul(std::string filename);

    std::string filename;
    std::vector<ast::top_level_ptr> top_lvl_items;

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
        std::vector<id_and_type> parameters;
        std::string return_type;
        uint32_t number;

        function_details(const std::vector<id_and_type> &, const std::optional<std::string> &,
                         uint32_t);

        function_details(std::vector<instruction> && instructions,
                         std::vector<id_and_type> && params, std::string && ret_type,
                         uint32_t number)
            : instructions{std::move(instructions)}
            , parameters{std::move(params)}
            , return_type{std::move(ret_type)}
            , number{number} {}
    };

    std::map<std::string, function_details> functions;
    std::string current_function;

    std::set<reg> used_registers;

    static constexpr uint32_t vm_text_start = 0x5000;
    static constexpr uint32_t vm_data_start = 0x4000;
    static constexpr uint32_t sp_start = 0x3000'0000;
    std::vector<uint8_t> data_segment;
    uint32_t func_num = 0;
};

struct compiled_expr {
    modul::reg reg_num;
    std::string type;
};
} // namespace bytecode

#endif
