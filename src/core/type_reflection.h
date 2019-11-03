//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <unordered_map>
#include <functional>
#include <exception>
#include <iostream>

#include <util/noncopyable.h>

#include "device.h"
#include "core_type.h"

namespace luisa {

struct TypeReflectionError : public std::runtime_error {
    template<typename ...Args>
    TypeReflectionError(std::string_view file, size_t line, Args &&...args) noexcept
        : std::runtime_error{util::serialize("TypeReflectionError: ", std::forward<Args>(args)..., "  [file: \"", file, "\", line: ", line, "]")} {}
};

#define THROW_TYPE_REFLECTION_ERROR(...)  throw TypeReflectionError{__FILE__, __LINE__, __VA_ARGS__}

class TypeReflectionManager : util::Noncopyable {

public:
    struct Info {
        std::string_view class_name;
        int32_t parent_index;
        CoreTypeTag base_tag;
        std::unordered_map<std::string_view, CoreTypeTag> properties{};
        
        Info(std::string_view cls, int32_t parent, CoreTypeTag tag) noexcept
            : class_name{cls}, parent_index{parent}, base_tag{tag} {}
    };
    
    using Creator = std::function<std::shared_ptr<CoreTypeBase>()>;

private:
    std::vector<Info> _info_list;
    std::unordered_map<std::string_view, int32_t> _class_ids;
    std::array<std::unordered_map<std::string_view, std::pair<std::string_view, Creator>>, non_value_core_type_count> _creators{};
    CoreTypeTag _curr_base_tag;
    TypeReflectionManager() = default;

public:
    TypeReflectionManager(TypeReflectionManager &&) = delete;
    TypeReflectionManager &operator=(TypeReflectionManager &&) = delete;
    
    [[nodiscard]] static TypeReflectionManager &instance() noexcept {
        static TypeReflectionManager manager;
        return manager;
    }
    
    int32_t register_class(std::string_view cls, CoreTypeTag base_tag, int32_t parent_id) noexcept {
        Info info{cls, parent_id, base_tag};
        for (auto p = parent_id; p != -1; p = _info_list[p].parent_index) {
            for (auto &&item : _info_list[p].properties) {
                info.properties.emplace(item.first, item.second);
            }
        }
        auto id = static_cast<int32_t>(_info_list.size());
        _info_list.emplace_back(std::move(info));
        _class_ids.emplace(cls, id);
        return id;
    }
    
    void register_property(std::string_view prop_name, CoreTypeTag prop_tag) noexcept {
        auto &&properties = _info_list.back().properties;
        assert(properties.count(prop_name) == 0);
        properties[prop_name] = prop_tag;
    }
    
    void register_creator(std::string_view detail_name, Creator creator) noexcept {
        auto base_tag_index = static_cast<uint32_t>(_info_list.back().base_tag);
        assert(base_tag_index < non_value_core_type_count && _creators[base_tag_index].count(detail_name) == 0);
        _creators[base_tag_index].emplace(detail_name, std::make_pair(_info_list.back().class_name, std::move(creator)));
    }
    
    [[nodiscard]] std::string_view derived_class_name(CoreTypeTag tag, std::string_view detail_name) {
        auto base_tag_index = static_cast<uint32_t>(_info_list.back().base_tag);
        assert(base_tag_index < non_value_core_type_count && _creators[base_tag_index].count(detail_name) != 0);
        return _creators[base_tag_index].at(detail_name).first;
    }
    
    [[nodiscard]] CoreTypeTag base_core_type_tag(std::string_view cls) const {
        auto iter = _class_ids.find(cls);
        if (iter == _class_ids.end()) {
            THROW_TYPE_REFLECTION_ERROR("unregistered class \"", cls, "\".");
        }
        return _info_list[iter->second].base_tag;
    }
    
    [[nodiscard]] CoreTypeTag property_tag(std::string_view cls, std::string_view prop) const {
        auto iter = _class_ids.find(cls);
        if (iter == _class_ids.end()) {
            THROW_TYPE_REFLECTION_ERROR("unregistered class \"", cls, "\".");
        }
        auto &&properties = _info_list[iter->second].properties;
        auto prop_iter = properties.find(prop);
        if (prop_iter == properties.end()) {
            THROW_TYPE_REFLECTION_ERROR("unregistered property \"", prop, "\" in class \"", cls, "\".");
        }
        return prop_iter->second;
    }
    
    [[nodiscard]] std::shared_ptr<CoreTypeBase> create(CoreTypeTag base_tag, std::string_view detail_name) {
        auto base_tag_index = static_cast<uint32_t>(base_tag);
        if (base_tag_index >= non_value_core_type_count) {
            THROW_TYPE_REFLECTION_ERROR("only non-value core types are supported in type reflection.");
        }
        auto &&creators = _creators[base_tag_index];
        auto iter = creators.find(detail_name);
        if (iter == creators.end()) {
            THROW_TYPE_REFLECTION_ERROR("no creator registered for type ", name_of_non_value_core_type_tag(base_tag), "::", detail_name, ".");
        }
        return iter->second.second();
    }
    
    void print() {
        for (auto &&item : _class_ids) {
            std::cout << item.first << " [id = " << item.second << "]\n";
            auto &&info = _info_list[item.second];
            std::cout << "  base_tag = " << name_of_core_type_tag(info.base_tag) << "\n";
            std::cout << "  parent_id = " << info.parent_index << "\n";
            if (info.parent_index != -1) {
                std::cout << "  parent_name = " << _info_list[info.parent_index].class_name << "\n";
            }
            for (auto &&prop_item : info.properties) {
                std::cout << "  property [name = " << prop_item.first << "]: " << name_of_core_type_tag(prop_item.second) << "\n";
            }
            std::cout << std::endl;
        }
    }
    
};

namespace _impl {

template<typename T>
struct TypeReflectionHelperImpl {};

template<>
struct TypeReflectionHelperImpl<CoreTypeBase> {
    static constexpr auto base_tag = CoreTypeTag::NON_VALUE_TYPE_COUNT;
    static constexpr auto id = -1;
};

}

template<CoreTypeTag tag>
struct WrapBaseCoreTypeTag {
    static constexpr auto base_tag = tag;
};

}

#define implements ,

#define DERIVED_CLASS(Cls, Parent)                                                                                                           \
    class Cls;                                                                                                                               \
    namespace _impl {                                                                                                                        \
        template<> struct TypeReflectionHelperImpl<Cls> {                                                                                    \
            static constexpr std::string_view name = #Cls;                                                                                   \
            static constexpr auto base_tag = [] {                                                                                            \
                if constexpr (std::is_same_v<Parent, CoreTypeBase>) {                                                                        \
                    return tag_of_non_value_core_type<Cls>;                                                                                  \
                } else {                                                                                                                     \
                    return TypeReflectionHelperImpl<Parent>::base_tag;                                                                       \
                }                                                                                                                            \
            }();                                                                                                                             \
            static inline auto id = TypeReflectionManager::instance().register_class(name, base_tag, TypeReflectionHelperImpl<Parent>::id);  \
            TypeReflectionHelperImpl() noexcept = default;                                                                                   \
        };                                                                                                                                   \
        inline TypeReflectionHelperImpl<Cls> _refl_##Cls{};                                                                                  \
    }                                                                                                                                        \
    class Cls                                                                                                                                \
        : public virtual Parent,                                                                                                             \
          public virtual WrapBaseCoreTypeTag<_impl::TypeReflectionHelperImpl<Cls>::base_tag>

#define CORE_CLASS(Cls)  \
    DERIVED_CLASS(Cls, CoreTypeBase)

#define PROPERTY(Type, name, tag)                                                                    \
        static_assert(true);                                                                         \
    private:                                                                                         \
        inline static struct _refl_##name##_helper {                                                 \
            _refl_##name##_helper() noexcept {                                                       \
                TypeReflectionManager::instance().register_property(#name, tag);                     \
            }                                                                                        \
        } _refl_##name{};                                                                            \
    protected:                                                                                       \
        Type _##name{};                                                                              \
        bool _decode_##name(const CoreTypeInitializerParameterSet &param_set) {                      \
            if (auto iter = param_set.find(#name); iter != param_set.end()) {                        \
                _decode_##name##_impl(std::get<std::vector<TypeOfCoreTypeTag<tag>>>(iter->second));  \
                return true;                                                                         \
            }                                                                                        \
            return false;                                                                            \
        }                                                                                            \
    private:                                                                                         \
        virtual void _decode_##name##_impl(const std::vector<TypeOfCoreTypeTag<tag>> &params)

#define CREATOR(detail_name)                                                              \
        static_assert(true);                                                              \
    private:                                                                              \
        inline static struct _refl_ctor_helper {                                          \
            _refl_ctor_helper() noexcept {                                                \
                TypeReflectionManager::instance().register_creator(detail_name, create);  \
            }                                                                             \
        } _refl_ctor{};                                                                   \
    public:                                                                               \
        [[nodiscard]] static TypeOfCoreTypeTag<base_tag> create()
        