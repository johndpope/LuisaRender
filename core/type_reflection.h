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

#include "core_type.h"

namespace luisa {

struct Empty {};  // convenience base type for core types

using CoreTypeCreatorParameterSet = std::unordered_map<std::string_view, CoreTypeVectorVariant>;

template<CoreTypeTag tag>
using CoreTypeCreator = std::function<TypeOfCoreTypeTag<tag>(const CoreTypeCreatorParameterSet &)>;

namespace _impl {

template<typename T>
struct TypeReflectionInfoImpl {};

template<>
struct TypeReflectionInfoImpl<Empty> {
    static constexpr std::string_view name{};
};

struct TypeReflectionRegistrationHelperImpl {
    
    static void register_class(std::string_view cls, std::string_view parent) noexcept;
    static void register_property(std::string_view prop, CoreTypeTag tag) noexcept;
    
    template<CoreTypeTag tag>
    static void register_creator(std::string_view name, CoreTypeCreator<tag> ctor) noexcept;
    
};

}

namespace _impl {

template<CoreTypeTag tag>
using MapCoreTypeTagToCreatorListImpl = std::unordered_map<std::string_view, CoreTypeCreator<tag>>;

}

using CoreTypeCreatorList = MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToCreatorListImpl>;

class TypeReflectionManager {

public:
    using PropertyList = std::unordered_map<std::string_view, CoreTypeTag>;

private:
    std::vector<std::string_view> _classes;
    std::unordered_map<std::string_view, std::string_view> _parents;
    std::unordered_map<std::string_view, PropertyList> _properties;
    PropertyList *_curr = nullptr;
    CoreTypeCreatorList _creators;
    
    friend _impl::TypeReflectionRegistrationHelperImpl;
    TypeReflectionManager() = default;

public:
    TypeReflectionManager(TypeReflectionManager &&) = delete;
    TypeReflectionManager(const TypeReflectionManager &) = delete;
    TypeReflectionManager &operator=(TypeReflectionManager &&) = delete;
    TypeReflectionManager &operator=(const TypeReflectionManager &) = delete;

public:
    [[nodiscard]] static TypeReflectionManager &instance() noexcept;
    [[nodiscard]] const PropertyList &properties(std::string_view cls) const noexcept;
    [[nodiscard]] std::string_view parent(std::string_view cls) const noexcept;
    [[nodiscard]] const std::vector<std::string_view> &classes() const noexcept;
    [[nodiscard]] bool is_core(std::string_view cls) const noexcept;
    [[nodiscard]] CoreTypeTag property_tag(std::string_view cls, std::string_view prop) const noexcept;
    [[nodiscard]] CoreTypeVariant create(const CoreTypeCreatorParameterSet &param, CoreTypeTag tag, std::string_view detail_name = "");
    [[nodiscard]] CoreTypeVariant create(const CoreTypeCreatorParameterSet &param, std::string_view base_type, std::string_view detail_name = "");
    
    template<typename T>
    [[nodiscard]] static constexpr std::string_view name() noexcept {
        return _impl::TypeReflectionInfoImpl<T>::name;
    }
    
    template<typename T>
    [[nodiscard]] static constexpr std::string_view parent() noexcept {
        return _impl::TypeReflectionInfoImpl<T>::parent_name;
    }
    
    template<typename T>
    [[nodiscard]] const PropertyList &properties() const noexcept {
        return properties(name<T>());
    }
    
    template<typename T>
    [[nodiscard]] CoreTypeTag property_tag(std::string_view prop) const noexcept {
        return property_tag(name<T>(), prop);
    }
    
    template<CoreTypeTag tag>
    [[nodiscard]] auto create(const CoreTypeCreatorParameterSet &param, std::string_view detail_name = "") {
        assert(!is_core_type_tag_value_type<tag>);
        auto &&ctors = std::get<index_of_core_type_tag<tag>>(_creators);
        assert(ctors.find(detail_name) != ctors.end());
        return ctors.at(detail_name)(param);
    }
};

namespace _impl {

template<typename Tuple>
struct TypeReflectionCreationHelperImpl {
    static auto create(const CoreTypeCreatorParameterSet &param, CoreTypeTag tag, std::string_view detail_name) -> CoreTypeVariant {
        return {};
    }
};

template<typename First, typename ...Others>
struct TypeReflectionCreationHelperImpl<std::tuple<First, Others...>> {
    static auto create(const CoreTypeCreatorParameterSet &param, CoreTypeTag tag, std::string_view detail_name) -> CoreTypeVariant {
        return tag == First::tag ?
               TypeReflectionManager::instance().create<First::tag>(param, detail_name) :
               TypeReflectionCreationHelperImpl<std::tuple<Others...>>::create(param, tag, detail_name);
    }
};

}

namespace _impl {

template<CoreTypeTag tag>
void TypeReflectionRegistrationHelperImpl::register_creator(std::string_view name, CoreTypeCreator<tag> ctor) noexcept {
    auto &m = TypeReflectionManager::instance();
    std::get<index_of_core_type_tag<tag>>(m._creators).emplace(name, std::move(ctor));
}

}

template<bool is_core>
struct WrapIsCoreType {
    static constexpr auto core_type = is_core;
};

template<CoreTypeTag tag>
struct WrapBaseTag {
    static constexpr auto base_tag = tag;
};

}

#define implements ,

#define derived_class(Cls, Parent)                                                                                  \
    class Cls;                                                                                                      \
    namespace _impl {                                                                                               \
        template<> struct TypeReflectionInfoImpl<Cls> {                                                             \
            static constexpr std::string_view name = #Cls;                                                          \
            static constexpr auto core_type = is_core_type<Cls>;                                                    \
            static_assert(core_type || name == name_of_core_type<Cls>, "Inconsistent core type name.");             \
            static constexpr auto base_tag = core_type ? tag_of_core_type<Cls> : base_tag_of_derived_type<Parent>;  \
            static constexpr std::string_view parent_name = TypeReflectionInfoImpl<Parent>::name;                   \
            TypeReflectionInfoImpl() noexcept {                                                                     \
                TypeReflectionRegistrationHelperImpl::register_class(name, parent_name);                            \
            }                                                                                                       \
        };                                                                                                          \
        inline TypeReflectionInfoImpl<Cls> _refl_##Cls{};                                                           \
    }                                                                                                               \
    class Cls                                                                                                       \
        : public virtual Parent,                                                                                    \
          public virtual WrapBaseTag<_impl::TypeReflectionInfoImpl<Cls>::base_tag>,                                 \
          public virtual WrapIsCoreType<_impl::TypeReflectionInfoImpl<Cls>::core_type>

#define core_class(Cls)  \
    derived_class(Cls, Empty)

#define property(type, name, tag)                                                                                                          \
        static_assert(true);                                                                                                               \
    private:                                                                                                                               \
        inline static struct _refl_##name##_helper {                                                                                       \
            _refl_##name##_helper() noexcept {                                                                                             \
                _impl::TypeReflectionRegistrationHelperImpl::register_property(#name, tag);                                                \
            }                                                                                                                              \
        } _refl_##name{};                                                                                                                  \
    protected:                                                                                                                             \
        type _##name{};                                                                                                                    \
    public:                                                                                                                                \
        void decode_##name(const CoreTypeCreatorParameterSet &param_set, const std::function<void(type &)> &default_op = [](type &) {}) {  \
            if (auto iter = param_set.find(#name); iter != param_set.end()) {                                                              \
                _decode_##name##_impl(std::get<std::vector<TypeOfCoreTypeTag<tag>>>(param_set.at(#name)));                                 \
            } else {                                                                                                                       \
                default_op(_##name);                                                                                                       \
            }                                                                                                                              \
        }                                                                                                                                  \
    private:                                                                                                                               \
        void _decode_##name##_impl(const std::vector<TypeOfCoreTypeTag<tag>> &params)

#define creator(detail_name)                                                                                             \
        static_assert(true);                                                                                             \
    private:                                                                                                             \
        inline static struct _refl_ctor_helper {                                                                         \
            _refl_ctor_helper() noexcept {                                                                               \
                _impl::TypeReflectionRegistrationHelperImpl::register_creator<base_tag>(detail_name, [] (auto &param) {  \
                    return create(param);                                                                                \
                });                                                                                                      \
            }                                                                                                            \
        } _refl_ctor{};                                                                                                  \
    public:                                                                                                              \
        [[nodiscard]] static TypeOfCoreTypeTag<tag> create(const CoreTypeCreatorParameterSet &param)
        