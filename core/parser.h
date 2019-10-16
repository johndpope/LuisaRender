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
                    throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): unexpected value \"", token, "\" for bool type (expected true or false).")};
                }
                _pop();
            } else if constexpr (tag == CoreTypeTag::FLOAT) {  // special handling for floats
                auto token = _peek();
                auto count = 0ul;
                auto value = stof(std::string{token}, &count);
                if (count != token.size()) {
                    throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): Failed to convert \"", token, "\" into float.")};
                }
                _pop();
                v.emplace_back(value);
            } else if constexpr (tag == CoreTypeTag::INTEGER) {  // special handling for integers
                auto token = _peek();
                auto count = 0ul;
                auto value = stoi(std::string{token}, &count);
                if (count != token.size()) {
                    throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): Failed to convert \"", token, "\" into integer.")};
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
        if constexpr (sizeof...(other_tags) == 0) {
            throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): unknown type to parser.")};
        } else {
            return _parse_property_setter_parameter_list(tag, std::tuple<WrapCoreTypeTag<other_tags>...>{});
        }
    }
    
    [[nodiscard]] CoreTypeVectorVariant _parse_property_setter_parameter_list(CoreTypeTag tag) {
        return _parse_property_setter_parameter_list(tag, CoreTypeTagList{});
    }
    
    [[nodiscard]] CoreTypeCreatorParameterSet _parse_creator_parameter_set(CoreTypeTag tag, std::string_view detail_type) {
        CoreTypeCreatorParameterSet param_set;
        _match("{");
        auto class_name = TypeReflectionManager::instance().derived_class_name(tag, detail_type);
        while (_peek() != "}") {
            auto property_name = _peek();
            if (param_set.find(property_name) != param_set.end()) {
                throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): duplicated property \"", property_name, "\".")};
            }
            auto property_tag = TypeReflectionManager::instance().property_tag(class_name, property_name);
            if (_peek() == "{") {  // setter list
                param_set.emplace(property_name, _parse_property_setter_parameter_list(property_tag));
            } else if (_peek() == ":") {  // convenience creation
                _pop();  // :
                auto property_detail_type = _peek();
                _pop();  // detail type
                auto property_creator_param_set = _parse_creator_parameter_set(property_tag, property_detail_type);
                auto property_instance = TypeReflectionManager::instance().create(property_tag, property_detail_type, property_creator_param_set);
                param_set.emplace(property_name, core_type_vector_variant_create(property_tag, property_instance));
            }
        }
        _match("}");
        return param_set;
    }

public:
    [[nodiscard]] std::vector<std::shared_ptr<Task>> parse(std::filesystem::path file_path);
    
};

}
