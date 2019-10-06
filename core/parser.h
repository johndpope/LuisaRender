//
// Created by Mike Smith on 2019/10/5.
//

#pragma once

#include <vector>
#include <variant>
#include <fstream>
#include <streambuf>

#include "type_reflection.h"
#include "string_manipulation.h"
#include "task.h"

namespace luisa {

class Parser {

private:
    size_t _curr_row;
    size_t _curr_col;
    std::string _source;
    std::unordered_map<std::string_view, CoreTypeVariant> _created;

public:
    [[nodiscard]] std::vector<std::shared_ptr<Task>> parse(std::filesystem::path file_path) {
        _curr_row = 0;
        _curr_col = 0;
        _source.clear();
        _created.clear();
        std::ifstream file{file_path};
        if (!file.is_open()) {
            throw std::runtime_error{serialize("Failed to open file: ", file_path)};
        }
        _source = std::string{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
        
    }

};

}
