//
// Created by Mike Smith on 2019/9/26.
//

#ifndef METALIGHT_RESOURCE_MANAGER_H
#define METALIGHT_RESOURCE_MANAGER_H

#include <filesystem>
#include <string>
#include <vector>

class ResourceManager {

public:
    using Path = std::filesystem::path;

private:
    Path _runtime_directory;
    Path _working_directory;
    
    ResourceManager()
        : _runtime_directory{std::filesystem::current_path()},
          _working_directory{std::filesystem::current_path()} {}

public:
    ResourceManager(ResourceManager &&) = delete;
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(ResourceManager &&) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;
    
    static ResourceManager &instance() noexcept {
        static ResourceManager manager;
        return manager;
    }
    
    void set_runtime_directory(const Path &p) {
        auto abs_path = std::filesystem::absolute(p);
        if (!std::filesystem::is_directory(abs_path)) {
            throw std::runtime_error{"Failed to set working directory: "};
        }
    }
    
};

#endif //METALIGHT_RESOURCE_MANAGER_H
