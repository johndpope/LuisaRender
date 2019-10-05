//
// Created by Mike Smith on 2019/10/4.
//

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>

#include "core_type.h"

namespace luisa {

struct Empty {};  // convenience base type for core types

using CoreTypeCreatorParameter = class ParserState;

template<CoreTypeTag tag>
using CoreTypeCreator = std::function<TypeOfCoreTypeTag<tag>(CoreTypeCreatorParameter &)>;

namespace _impl {

template<typename T>
struct TypeReflectionInfoImpl {};

template<>
struct TypeReflectionInfoImpl<Empty> {
    static constexpr std::string_view name{};
};

struct TypeReflectionHelperImpl {
    
    static void register_class(std::string_view cls, std::string_view parent) noexcept;
    static void register_property(std::string_view prop, CoreTypeTag tag) noexcept;
    
    template<CoreTypeTag tag>
    static void register_creator(std::string_view name, CoreTypeCreator<tag> ctor) noexcept;
    
};

template<typename Tuple, typename Result>
struct CoreTypeCreatorListImpl {
    using Type = Result;
};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo, typename ...Ctors>
struct CoreTypeCreatorListImpl<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, std::tuple<Ctors...>> {
    using Type = typename CoreTypeCreatorListImpl<
        std::tuple<OtherInfo...>,
        std::tuple<Ctors..., std::unordered_map<std::string_view, CoreTypeCreator<first_tag>>>>::Type;
};

}

using CoreTypeCreatorList = typename _impl::CoreTypeCreatorListImpl<AllCoreTypeInfo, std::tuple<>>::Type;

class TypeReflectionManager {

public:
    using PropertyList = std::unordered_map<std::string_view, CoreTypeTag>;

private:
    std::vector<std::string_view> _classes;
    std::unordered_map<std::string_view, std::string_view> _parents;
    std::unordered_map<std::string_view, PropertyList> _properties;
    PropertyList *_curr = nullptr;
    CoreTypeCreatorList _creators;
    
    friend _impl::TypeReflectionHelperImpl;
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
    [[nodiscard]] CoreTypeTag property_tag(std::string_view cls, std::string_view prop) const noexcept;
    [[nodiscard]] bool is_core(std::string_view cls) const noexcept;
    
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
    [[nodiscard]] auto create(std::string_view detail_name, CoreTypeCreatorParameter &param) {
        auto &&ctors = std::get<index_of_core_type_tag<tag>>(_creators);
        assert(ctors.find(detail_name) != ctors.end());
        return ctors.at(detail_name)(param);
    }
    
};

namespace _impl {

template<CoreTypeTag tag>
void TypeReflectionHelperImpl::register_creator(std::string_view name, CoreTypeCreator<tag> ctor) noexcept {
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

#define derived_class(Cls, Parent)                                                                                          \
    class Cls;                                                                                                              \
    namespace _impl {                                                                                                       \
        template<> struct TypeReflectionInfoImpl<Cls> {                                                                     \
            static constexpr std::string_view name = #Cls;                                                                  \
            static constexpr auto core_type = is_core_type<Cls>;                                                            \
            static_assert(core_type || name == name_of_core_type<Cls>, "Inconsistent core type name.");                     \
            static constexpr auto base_tag = core_type ? tag_of_core_type<Cls> : base_tag_of_derived_type<Parent>;          \
            static constexpr std::string_view parent_name = TypeReflectionInfoImpl<Parent>::name;                           \
            TypeReflectionInfoImpl() noexcept {                                                                             \
                TypeReflectionHelperImpl::register_class(name, parent_name);                                                \
            }                                                                                                               \
        };                                                                                                                  \
        inline TypeReflectionInfoImpl<Cls> _refl_##Cls{};                                                                   \
    }                                                                                                                       \
    class Cls                                                                                                               \
        : public virtual Parent,                                                                                            \
          public virtual WrapBaseTag<_impl::TypeReflectionInfoImpl<Cls>::base_tag>,                                         \
          public virtual WrapIsCoreType<_impl::TypeReflectionInfoImpl<Cls>::core_type>

#define core_class(Cls)  \
    derived_class(Cls, Empty)

#define property(type, name, tag)                                                \
        inline static struct _refl_##name##_helper {                             \
            _refl_##name##_helper() noexcept {                                   \
                _impl::TypeReflectionHelperImpl::register_property(#name, tag);  \
            }                                                                    \
        } _refl_##name{};                                                        \
    public:                                                                      \
        [[nodiscard]] const type &name() const noexcept { return _##name; }      \
        void set_##name(const type &v) noexcept { _##name = v; }                 \
    protected:                                                                   \
        type _##name

#define creator(detail_name)                                                                                 \
        inline static struct _refl_ctor_helper {                                                             \
            _refl_ctor_helper() noexcept {                                                                   \
                _impl::TypeReflectionHelperImpl::register_creator<base_tag>(detail_name, [] (auto &param) {  \
                    return create(param);                                                                    \
                });                                                                                          \
            }                                                                                                \
        } _refl_ctor{};                                                                                      \
    public:                                                                                                  \
        static TypeOfCoreTypeTag<tag> create(CoreTypeCreatorParameter &param)
        