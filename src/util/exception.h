//
// Created by Mike Smith on 2019/11/4.
//

#pragma once

#include <exception>
#include <iostream>

#include "string_manipulation.h"

#define LUISA_MAKE_ERROR_TYPE(Type)                                                                                                           \
    struct Type : public std::runtime_error {                                                                                                 \
        template<typename ...Args>                                                                                                            \
        Type(std::string_view file, size_t line, Args &&...args) noexcept                                                                     \
            : std::runtime_error{util::serialize(#Type, ": ", std::forward<Args>(args)..., "  [file: \"", file, "\", line: ", line, "]")} {}  \
    }                                                                                                                                         \
    
#define LUISA_THROW_ERROR(Type, ...)  \
    throw Type{__FILE__, __LINE__, __VA_ARGS__}
    
#define LUISA_WARNING(...)  \
    std::cerr << "Warning: " << util::serialize(__VA_ARGS__) << "  [file: \"" << __FILE__ << "\" << line: " << __LINE__ << "]" << std::endl
    