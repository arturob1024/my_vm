#ifndef MODULE_H
#define MODULE_H

#include "ast/nodes_forward.h"

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

class modul;
using module_and_file = std::pair<std::unique_ptr<modul>, FILE *>;
using id_and_type = std::pair<std::string, std::string>;

struct compiled_expr {
    uint8_t reg_num;
    std::string type;
};

class modul final {

  public:
    static module_and_file open_module(const char * path);
    static module_and_file open_stdin();

    void build();

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

    compiled_expr compile_binary_op(ast::binary_operation, compiled_expr, compiled_expr) {
        return {};
    }

    modul(const modul &) = delete;
    modul & operator=(const modul &) = delete;

    modul(modul &&) noexcept = default;
    modul & operator=(modul &&) noexcept = default;

    ~modul() noexcept = default;

  private:
    enum reg : uint8_t {
        zero,
        v0,
        v1,
        a0,
        a1,
        a2,
        a3,
        a4,
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

    explicit modul(std::string filename)
        : filename{std::move(filename)} {
        functions.emplace(
            "print", function_details{
                         std::vector{instruction{opcode::ori, i_type{temp, zero, 3}},
                                     instruction{opcode::syscall, s_type{temp, a0, zero, zero, 1}},
                                     instruction{opcode::jr, j_type{lr, 0}}},
                         std::vector{id_and_type{"input", "string"}}, "", func_num++});
    }

    std::string filename;
    std::vector<ast::top_level_ptr> top_lvl_items;

    enum class func_num : uint8_t {

    };

    enum class opcode : uint8_t {
        r_type = 0,
        lui = 1,
        ori = 5,
        jal = 20,
        jr = 21,
        syscall = 63,
    };

    struct r_type {};
    struct i_type {
        uint8_t rd;
        uint8_t rs;
        uint16_t imm;
    };
    struct j_type {
        uint8_t rd;
        uint32_t imm;
    };
    struct s_type {
        uint8_t rd;
        uint8_t rs1;
        uint8_t rs2;
        uint8_t rs3;
        uint8_t func;
    };

    using instruction_data = std::variant<r_type, i_type, j_type, s_type>;
    struct instruction {
        opcode op;
        instruction_data data;

        instruction(opcode op, instruction_data && data)
            : op{op}
            , data{std::move(data)} {}
    };

    void add_instruction(opcode, instruction_data &&);
    uint8_t alloc_reg();

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

    static constexpr uint32_t data_segment_start = 0x4000;
    std::vector<uint8_t> data_segment;
    uint32_t func_num = 0;
};

#endif
