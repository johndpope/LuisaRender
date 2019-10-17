//
// Created by Mike Smith on 2019/10/5.
//

#include "parser.h"

namespace luisa {

void Parser::_skip_blanks_and_comments() {
    if (!_peeked.empty()) {
        throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): peeked token \"", _peeked, "\" should not be skipped.")};
    }
    _peeked = {};
    while (!_remaining.empty()) {
        if (_remaining.front() == '\r') {
            _remaining = _remaining.substr(1);
            if (!_remaining.empty() && _remaining.front() == '\n') {
                _remaining = _remaining.substr(1);
            }
            _next_line++;
            _next_col = 0;
        } else if (_remaining.front() == '\n') {
            _remaining = _remaining.substr(1);
            _next_line++;
            _next_col = 0;
        } else if (std::isblank(_remaining.front())) {
            _remaining = _remaining.substr(1);
            _next_col++;
        } else if (_remaining.front() == '/') {
            _remaining = _remaining.substr(1);
            _next_col++;
            if (_remaining.empty() || _remaining.front() != '/') {
                throw std::runtime_error{serialize("ParserError (Line ", _next_line, ", Col ", _next_col, "): expected '/'.")};
            }
            for (; !_remaining.empty() && _remaining.front() != '\r' && _remaining.front() != '\n'; _remaining = _remaining.substr(1)) {
                _next_col++;
            }
        } else {
            break;
        }
    }
    _curr_line = _next_line;
    _curr_col = _next_col;
}

std::string_view Parser::_peek() {
    if (_peeked.empty()) {
        if (_remaining.empty()) {
            throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): peek at the end of the file.")};
        }
        if (_remaining.front() == '{' || _remaining.front() == '}' || _remaining.front() == ':' || _remaining.front() == ',' || _remaining.front() == '@') {  // symbols
            _peeked = _remaining.substr(0, 1);
            _remaining = _remaining.substr(1);
            _next_col++;
        } else if (_remaining.front() == '_' || std::isalpha(_remaining.front())) {  // Keywords or identifiers
            auto i = 1ul;
            for (; i < _remaining.size() && (_remaining[i] == '_' || std::isalnum(_remaining[i])); i++) {}
            _peeked = _remaining.substr(0, i);
            _remaining = _remaining.substr(i);
            _next_col += i;
        } else if (_remaining.front() == '+' || _remaining.front() == '-' || _remaining.front() == '.' || std::isdigit(_remaining.front())) {  // numbers
            auto i = 1ul;
            for (; i < _remaining.size() && (_remaining[i] == '.' || std::isdigit(_remaining[i])); i++) {}
            _peeked = _remaining.substr(0, i);
            _remaining = _remaining.substr(i);
            _next_col += i;
        } else if (_remaining.front() == '"') {  // strings
            auto i = 1ul;
            for (; i < _remaining.size() && _remaining[i] != '"' && _remaining[i] != '\r' && _remaining[i] != '\n'; i++) {
                if (_remaining[i] == '\\') { i++; }  // dirty handling for escape characters
            }
            _next_col += i + 1;
            if (i >= _remaining.size() || _remaining[i] != '"') {
                throw std::runtime_error{serialize("ParserError (Line ", _next_line, ", Col ", _next_col, "): expected '\"'.")};
            }
            _peeked = _remaining.substr(0, i + 1);
            _remaining = _remaining.substr(i + 1);
        }
    }
    return _peeked;
}

void Parser::_pop() {
    if (_peeked.empty()) {
        throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): token not peeked before being popped.")};
    }
    _peeked = {};
    _curr_line = _next_line;
    _curr_col = _next_col;
    _skip_blanks_and_comments();
}

std::vector<std::shared_ptr<Task>> Parser::_parse_top_level() {
    std::vector<std::shared_ptr<Task>> tasks;
    while (!_finished()) {
        if (_peek() == "tasks") {
            _pop();  // tasks
            tasks = _parse_property_setter_parameter_list_impl<CoreTypeTag::TASK>();
            if (!_finished()) {
                throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): tasks should be defined at the end of the file.")};
            }
            break;
        } else {
            auto type = _peek();
            _pop();  // type
            auto tag = tag_of_core_type_name(type);
            auto name = _peek();
            _pop();  // name
            if (_created.find(name) != _created.end()) {
                throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): duplicated object name \"", name, "\".")};
            }
            _match(":");
            auto detail_type = _peek();
            _pop();  // detail type
            auto param_set = _parse_creator_parameter_set(tag, detail_type);
            auto object = TypeReflectionManager::instance().create(tag, detail_type, param_set);
            _created.emplace(name, object);
        }
    }
    return tasks;
}

void Parser::_match(std::string_view token) {
    if (_peek() != token) {
        throw std::runtime_error{serialize("ParserError (Line ", _curr_line, ", Col ", _curr_col, "): expected \"", token, "\", got \"", _peek(), "\".")};
    }
    _pop();
}

std::vector<std::shared_ptr<Task>> Parser::parse(std::filesystem::path file_path) {
    _curr_line = 0;
    _curr_col = 0;
    _next_line = 0;
    _next_col = 0;
    _source.clear();
    _created.clear();
    _peeked = {};
    _remaining = {};
    std::ifstream file{file_path};
    if (!file.is_open()) {
        throw std::runtime_error{serialize("Failed to open file: ", file_path)};
    }
    _source = std::string{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    _remaining = _source;
    _skip_blanks_and_comments();
    return _parse_top_level();
}

bool Parser::_finished() const noexcept {
    return _peeked.empty() && _remaining.empty();
}

}