//
// Created by Mike Smith on 2019-08-13.
//

#ifndef METALIGHT_RESOURCE_H
#define METALIGHT_RESOURCE_H

#include <string>
#include <string_view>

inline std::string get_resource_path(std::string_view name) noexcept {
    static constexpr std::string_view base{RESOURCE_DIR};
    static constexpr std::string_view sep{(base.empty() || base.back() == '/') ? "" : "/"};
    return std::string{base}.append(sep).append(name);
}

#endif //METALIGHT_RESOURCE_H
