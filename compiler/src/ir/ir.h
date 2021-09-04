#ifndef IR_H
#define IR_H

#include "ast/nodes_forward.h"
#include "bytecode/module_forward.h"
#include "ir_forward.h"

#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ir {

struct operand {
    std::string name;
    // TODO: Use a type type
    std::string type;
};

struct instruction {
    operation op;
    // TODO: shared_ptr?
    std::vector<operand> args;
    std::optional<operand> result;

    instruction(operation op, std::vector<operand> args, std::optional<operand> result)
        : op{op}
        , args{std::move(args)}
        , result{std::move(result)} {}

    friend std::ostream & operator<<(std::ostream &, const instruction &);
};

struct modul {
  public:
    // Top level item compilation

    void register_global(std::string, std::optional<std::string>, ast::expression &, bool constant);
    void register_function(std::string id, const std::vector<ast::typed_id> & params,
                           const std::optional<std::string> & type, ast::statement & body);

    void register_struct(std::string id, const std::vector<ast::typed_id> & params);

    // Statment compilation

    void call_function(std::string id, std::vector<operand> args);

    // Expression compilation

    operand compile_literal(const std::string & value, ast::type typ);

    operand compile_binary_op(ast::binary_operation, operand, operand);

    explicit modul(std::string filename)
        : filename{std::move(filename)} {}

    modul(const modul &) = delete;
    modul & operator=(const modul &) = delete;

    modul(modul &&) noexcept = default;
    modul & operator=(modul &&) noexcept = default;

    ~modul() noexcept = default;

  private:
    struct function_details {
        std::vector<instruction> instructions;
        std::vector<bytecode::id_and_type> parameters;
        std::string return_type;
        uint32_t number;

        function_details(const std::vector<bytecode::id_and_type> &,
                         const std::optional<std::string> &, uint32_t);

        function_details(std::vector<instruction> && instructions,
                         std::vector<bytecode::id_and_type> && params, std::string && ret_type,
                         uint32_t number)
            : instructions{std::move(instructions)}
            , parameters{std::move(params)}
            , return_type{std::move(ret_type)}
            , number{number} {}
    };

    [[nodiscard]] function_details & current_function();
    [[nodiscard]] operand temp_operand(std::string);

    std::map<std::string, function_details> functions;
    std::string current_func_name;

    std::string filename;
    int temp_num = 0;
    uint32_t func_num = 0;

    friend std::ostream & operator<<(std::ostream &, const ir::modul &);
};

} // namespace ir

#endif
