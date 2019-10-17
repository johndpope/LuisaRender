//
// Created by Mike Smith on 2019/10/5.
//

#pragma once

#include <vector>
#include <variant>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <charconv>

#include "type_reflection.h"
#include "string_manipulation.h"
#include "task.h"

namespace luisa {

struct ParserError : std::runtime_error {
    template<typename ...Args>
    ParserError(std::string_view file, size_t line, size_t curr_line, size_t curr_col, Args &&...args) noexcept
        : std::runtime_error{serialize("ParserError (line ", curr_line, ", col ", curr_col, "): ", std::forward<Args>(args)..., "  [file: \"", file, "\", line: ", line, "]")} {}
};

#define THROW_PARSER_ERROR(...) throw ParserError{__FILE__, __LINE__, __VA_ARGS__}

class Parser {

private:
    size_t _curr_line{};
    size_t _curr_col{};
    size_t _next_line{};
    size_t _next_col{};
    std::string _source;
    std::string_view _peeked;
    std::string_view _remaining;
    std::unordered_map<std::string_view, CoreTypeVariant> _created;
    
    void _skip_blanks_and_comments();
    [[nodiscard]] std::string_view _peek();
    void _pop();
    void _match(std::string_view token);
    [[nodiscard]] std::vector<std::shared_ptr<Task>> _parse_top_level();
    [[nodiscard]] bool _finished() const noexcept;
    
    template<CoreTypeTag tag>
    [[nodiscard]] auto _parse_property_setter_parameter_list_impl() {
        
        using Type = TypeOfCoreTypeTag<tag>;
        std::vector<Type> v;
        _match("{");
        while (_peek() != "}") {
            if constexpr (tag == CoreTypeTag::STRING) {  // special handling for strings
            
            } else if constexpr (tag == CoreTypeTag::BOOL) {  // special handling for bools
                auto token = _peek();
                if (token == "true") {
                    v.emplace_back(true);
                } else if (token == "false") {
                    v.emplace_back(false);
                } else {
                    THROW_PARSER_ERROR(_curr_line, _curr_col, "unexpected value \"", token, "\" for bool type (expected true or false).");
                }
                _pop();
            } else if constexpr (tag == CoreTypeTag::FLOAT) {  // special handling for floats
                auto token = _peek();
                auto count = 0ul;
                auto value = stof(std::string{token}, &count);
                if (count != token.size()) {
                    THROW_PARSER_ERROR(_curr_line, _curr_col, "failed to convert \"", token, "\" into float.");
                }
                _pop();
                v.emplace_back(value);
            } else if constexpr (tag == CoreTypeTag::INTEGER) {  // special handling for integers
                auto token = _peek();
                auto count = 0ul;
                auto value = stoi(std::string{token}, &count);
                if (count != token.size()) {
                    THROW_PARSER_ERROR(_curr_line, _curr_col, "failed to convert \"", token, "\" into integer.");
                }
                _pop();
                v.emplace_back(value);
            } else {  // non-value core types
                if (_peek() == "@") {  // reference
                    _pop();  // @
                    auto element_name = _peek();
                    _pop();  // name
                    v.emplace_back(std::get<Type>(_created.at(element_name)));
                } else {  // inline creation
                    auto element_detail_type = _peek();
                    _pop();  // detail type
                    auto element_parameter_set = _parse_creator_parameter_set(tag, element_detail_type);
                    auto element_instance = TypeReflectionManager::instance().create(tag, element_detail_type, element_parameter_set);
                    v.emplace_back(std::get<Type>(element_instance));
                }
            }
            if (_peek() != "}") { _match(","); }
        }
        _match("}");
        return v;
    }
    
    template<CoreTypeTag first_tag, CoreTypeTag ...other_tags>
    [[nodiscard]] CoreTypeVectorVariant _parse_property_setter_parameter_list(CoreTypeTag tag, std::tuple<WrapCoreTypeTag<first_tag>, WrapCoreTypeTag<other_tags>...>) {
        if (tag == first_tag) { return _parse_property_setter_parameter_list_impl<first_tag>(); }
        if constexpr (sizeof...(other_tags) != 0) {
            return _parse_property_setter_parameter_list(tag, std::tuple<WrapCoreTypeTag<other_tags>...>{});
        }
        THROW_PARSER_ERROR(_curr_line, _curr_col, "unknown type tag to parse.");
    }
    
    [[nodiscard]] CoreTypeVectorVariant _parse_property_setter_parameter_list(CoreTypeTag tag);
    [[nodiscard]] CoreTypeCreatorParameterSet _parse_creator_parameter_set(CoreTypeTag tag, std::string_view detail_type);

public:
    [[nodiscard]] std::vector<std::shared_ptr<Task>> parse(std::filesystem::path file_path);
    
};

}
