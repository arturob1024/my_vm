#include "module.h"

#include "ast/nodes.h"
#include "ir/ir.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>

namespace bytecode {

modul::modul(std::string filename)
    : filename{std::move(filename)} {}

void modul::build() {}

void modul::add_instruction(opcode op, std::variant<r_type, i_type, j_type, s_type> && data) {

    auto iter = functions.find(current_function);
    assert(iter != functions.end());

    iter->second.instructions.emplace_back(op, std::move(data));
}

modul::reg modul::alloc_reg() {
    auto candidate = static_cast<reg>(s0 + random() % (s19 - s0 + 1));
    auto used_regs = used_registers();
    while (used_regs.count(candidate) != 0)
        candidate = static_cast<reg>(s0 + random() % (s19 - s0 + 1));
    return candidate;
}

void modul::write() {
    auto output_name = filename;
    output_name.erase(output_name.rfind('.') + 1);
    output_name += "bin";
    auto * output = fopen(output_name.c_str(), "w");

    static constexpr uint8_t magic_bytes[]{0xEF, 0x12, 0x34, 0x56, 0x78, 0x9A,
                                           0xBC, 0xDE, 1,    0,    0,    0};
    static_assert(sizeof(magic_bytes) == 12);
    // primary header
    if (fwrite(magic_bytes, 1, sizeof(magic_bytes), output) != sizeof(magic_bytes)) {
        std::cout << "Error occured writing out binary." << std::endl;
        exit(10);
    }

    auto prog_data = layout_segments(sizeof(magic_bytes) + sizeof(uint32_t) * 3);
    if (fwrite(&prog_data.exec_start, sizeof(prog_data.exec_start), 1, output) != 1) {
        std::cout << "Error occured writing out binary." << std::endl;
        exit(10);
    }

    static_assert(sizeof(sp_start) == 4);
    if (fwrite(&sp_start, sizeof(sp_start), 1, output) != 1) {
        std::cout << "Error occured writing out binary." << std::endl;
        exit(10);
    }
    // segment table
    static constexpr auto segment_table_item_size = sizeof(prog_data.segment_table.front());
    assert(prog_data.segment_table.size() < UINT32_MAX / segment_table_item_size);
    uint32_t segment_table_byte_len
        = static_cast<uint32_t>(prog_data.segment_table.size() * segment_table_item_size);
    static_assert(sizeof(segment_table_byte_len) == 4);

    if (fwrite(&segment_table_byte_len, sizeof(segment_table_byte_len), 1, output) != 1) {
        std::cout << "Error occured writing out binary." << std::endl;
        exit(10);
    }

    if (fwrite(prog_data.segment_table.data(), segment_table_item_size,
               prog_data.segment_table.size(), output)
        != prog_data.segment_table.size()) {
        std::cout << "Error occured writing out binary." << std::endl;
        exit(10);
    }

    // actual code
    if (fwrite(prog_data.segment_data.data(), sizeof(prog_data.segment_data.front()),
               prog_data.segment_data.size(), output)
        != prog_data.segment_data.size()) {
        std::cout << "Error occured writing out binary." << std::endl;
        exit(10);
    }

    fclose(output);
}

modul::program_data modul::layout_segments(uint32_t start_segment_table) {
    std::vector<uint32_t> segment_table;
    std::vector<uint32_t> segment_data;

    struct segment {
        uint32_t start_after_table;
        uint32_t length;
        uint32_t vm_addr;
        std::string name;

        uint32_t size() const {
            auto name_size = name.size() + 1;
            while (name_size % 4 != 0) ++name_size;
            return static_cast<uint32_t>(sizeof(uint32_t) * 3 + name_size);
        }
    };
    std::vector<segment> segments;

    while (data_segment.size() % 4 != 0) data_segment.push_back(0);
    assert(data_segment.size() < UINT32_MAX);
    // data segment
    for (auto i = 0u; i < data_segment.size(); i += 4)
        segment_data.push_back(data_segment[i] << (32 - 8) | data_segment[i + 1] << 16
                               | data_segment[i + 2] << 8 | data_segment[i + 3]);

    auto text_start = static_cast<uint32_t>(data_segment.size());
    segments.push_back({0, text_start, vm_data_start, ".data"});
    // text segment
    std::vector<function_details> funcs;
    for (auto & iter : functions) funcs.push_back(iter.second);
    std::sort(funcs.begin(), funcs.end(),
              [](const function_details & lhs, const function_details & rhs) {
                  return lhs.number < rhs.number;
              });
    const auto main_num = functions.find("main")->second.number;

    std::map<uint32_t, uint32_t> func_addrs;
    for (auto & func : funcs) {
        func_addrs.insert({func.number, vm_text_start + segment_data.size() * 4 - text_start});

        for (auto & instruction : func.instructions) {
            // fill in jal info
            if (instruction.op == opcode::jal) {
                auto & data = std::get<j_type>(instruction.data);
                data.imm = func_addrs.find(data.imm)->second;
            }
            segment_data.push_back(instruction);
        }
        // TODO: Add this earlier in the pipeline
        if (func.number == main_num)
            segment_data.push_back(
                instruction{opcode::syscall, s_type{zero, zero, zero, zero, zero}});
    }

    segments.push_back({text_start, static_cast<uint32_t>(segment_data.size() * 4) - text_start,
                        vm_text_start, ".text"});

    auto segment_table_total_size
        = std::accumulate(segments.begin(), segments.end(), 0u,
                          [](uint32_t sum, const auto & segment) { return sum + segment.size(); });
    for (auto & segment : segments) {
        segment_table.push_back(start_segment_table + segment_table_total_size
                                + segment.start_after_table);
        segment_table.push_back(segment.length);
        segment_table.push_back(segment.vm_addr);
        uint32_t temp = 0;
        auto i = 0ul;
        for (; i < segment.name.size(); ++i) {
            if (i != 0 and i % 4 == 0) {
                segment_table.push_back(temp);
                temp = 0;
            }
            temp <<= 8;
            temp |= (i < segment.name.size() ? segment.name.at(i) : 0);
        }
        if ((segment.name.size() + 1) % 4 != 0) {
            while (i % 4 != 0) {
                temp <<= 8;
                ++i;
            }
            segment_table.push_back(temp);
        }
    }

    return {segment_table, segment_data, func_addrs.find(main_num)->second};
}

const modul::function_details & modul::cur_func() const {
    assert(not current_function.empty());
    auto iter = functions.find(current_function);
    assert(iter != functions.end());
    return iter->second;
}

std::set<modul::reg> modul::used_registers() const {
    std::set<reg> used;
    for (auto & iter : cur_func().allocated_registers) {
        // TODO: Use lifetime analysis
        used.insert(iter.second);
    }
    return used;
}

[[nodiscard]] modul::instruction::operator uint32_t() const {
    // TODO: Name magic numbers
    uint32_t result = (uint32_t)op << 26;
    switch (op) {
    case opcode::r_type: {
        auto data = std::get<r_type>(this->data);
        result |= (data.rd << 21) | (data.rs1 << 16) | (data.rs2 << 11) | (data.shamt << 6)
                | (uint8_t)data.func;
    } break;
    case opcode::lui:
    case opcode::ori: {
        auto data = std::get<i_type>(this->data);
        result |= (data.rd << 21) | (data.rs << 16) | data.imm;
    } break;
    case opcode::jal:
    case opcode::jr: {
        auto data = std::get<j_type>(this->data);
        result |= (data.rd << 21) | ((data.imm >> 2) & 0x1F'FFFF);
    } break;
    case opcode::syscall: {
        auto data = std::get<s_type>(this->data);
        result
            |= (data.rd << 21) | (data.rs1 << 16) | (data.rs2 << 11) | (data.rs3 << 6) | data.func;
    } break;
    }
    return result;
}
} // namespace bytecode
