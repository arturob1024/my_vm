#ifndef MODULE_H
#define MODULE_H

#include "ast/nodes_forward.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

class modul;
using module_and_file = std::pair<std::unique_ptr<modul>, FILE *>;

class modul final {

  public:
    static module_and_file open_module(const char * path);
    static module_and_file open_stdin();

    void add_top_level_item(ast::top_level * top_lvl);

    [[nodiscard]] size_t top_level_item_count() const noexcept { return top_lvl_items.size(); }

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
};

#endif
