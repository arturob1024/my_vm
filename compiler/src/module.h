#ifndef MODULE_H
#define MODULE_H

#include "ast/nodes_forward.h"

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class modul;
using module_and_file = std::pair<std::unique_ptr<modul>, FILE *>;

class modul final {

  public:
    static module_and_file open_module(const char * path);
    static module_and_file open_stdin();

    void build();

    void add_top_level_item(ast::top_level * top_lvl);

    [[nodiscard]] size_t top_level_item_count() const noexcept { return top_lvl_items.size(); }

    void register_global(std::string id, const std::optional<std::string> & type,
                         ast::expression & value, bool constant);

    void register_function(std::string id, const std::vector<ast::typed_id> & params,
                           const std::optional<std::string> & type, ast::statement & body);

    void register_struct(std::string id, const std::vector<ast::typed_id> & params);

    void call_function(std::string id);

    modul(const modul &) = delete;
    modul & operator=(const modul &) = delete;

    modul(modul &&) noexcept = default;
    modul & operator=(modul &&) noexcept = default;

    ~modul() noexcept = default;

  private:
    explicit modul(std::string filename)
        : filename{std::move(filename)} {}

    std::string filename;
    std::vector<ast::top_level_ptr> top_lvl_items;

    enum class opcode : uint8_t {};

    struct instruction {
        opcode op;
        // TODO: Use an union?
    };

    struct function_details {
        std::vector<instruction> instructions;
        std::map<std::string, std::string> parameters;
        std::string return_type;

        function_details(const std::vector<ast::typed_id> &, const std::optional<std::string> &);
    };

    std::map<std::string, function_details> functions;
    std::string current_function;
};

#endif
