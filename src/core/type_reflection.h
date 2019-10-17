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

template<CoreTypeTag tag>
using MapCoreTypeTagToDerivedTypeClassNameListImpl = std::unordered_map<std::string_view, std::string_view>;

}

using CoreTypeCreatorList = MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToCreatorListImpl>;
using DerivedTypeClassNameList = MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToDerivedTypeClassNameListImpl>;

class TypeReflectionManager {

public:
    using PropertyList = std::unordered_map<std::string_view, CoreTypeTag>;

private:
    std::vector<std::string_view> _classes;
    std::unordered_map<std::string_view, std::string_view> _parents;
    std::unordered_map<std::string_view, PropertyList> _properties;
    PropertyList *_curr = nullptr;
    CoreTypeCreatorList _creators;
    DerivedTypeClassNameList _derived_classes;
    
    friend _impl::TypeReflectionRegistrationHelperImpl;
    TypeReflectionManager() = default;
    
    template<CoreTypeTag first_tag, CoreTypeTag ...other_tags>
    [[nodiscard]] std::string_view _derived_class_name_impl(
        CoreTypeTag tag, std::string_view detail_name,
        std::tuple<WrapCoreTypeTag<first_tag>, WrapCoreTypeTag<other_tags>...>) const noexcept {
        if (tag == first_tag) {
            return derived_class_name<first_tag>(detail_name);
        }
        if constexpr (sizeof...(other_tags) != 0) {
            return _derived_class_name_impl(tag, detail_name, std::tuple<WrapCoreTypeTag<other_tags>...>{});
        }
        assert(false);
        return {};
    }

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
    [[nodiscard]] CoreTypeVariant create(CoreTypeTag tag, std::string_view detail_name, const CoreTypeCreatorParameterSet &param);
    [[nodiscard]] CoreTypeVariant create(std::string_view base_type, std::string_view detail_name, const CoreTypeCreatorParameterSet &param);
    [[nodiscard]] std::string_view derived_class_name(CoreTypeTag tag, std::string_view detail_name) const noexcept;
    
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
    [[nodiscard]] auto create(std::string_view detail_name, const CoreTypeCreatorParameterSet &param) {
        assert(!is_core_type_tag_value_type<tag>);
        auto &&ctors = std::get<index_of_core_type_tag<tag>>(_creators);
        assert(ctors.find(detail_name) != ctors.end());
        return ctors.at(detail_name)(param);
    }
    
    template<CoreTypeTag tag>
    [[nodiscard]] auto derived_class_name(std::string_view detail_name) const noexcept {
        assert(!is_core_type_tag_value_type<tag>);
        auto &&class_list = std::get<index_of_core_type_tag<tag>>(_derived_classes);
        assert(class_list.find(detail_name) != class_list.end());
        return class_list.at(detail_name);
    }
};

namespace _impl {

template<typename Tuple>
struct TypeReflectionCreationHelperImpl {
    static CoreTypeVariant create(CoreTypeTag, std::string_view, const CoreTypeCreatorParameterSet &) { return {}; }
};

template<typename First, typename ...Others>
struct TypeReflectionCreationHelperImpl<std::tuple<First, Others...>> {
    static auto create(CoreTypeTag tag, std::string_view detail_name, const CoreTypeCreatorParameterSet &param) -> CoreTypeVariant {
        return tag == First::tag ?
               TypeReflectionManager::instance().create<First::tag>(detail_name, param) :
               TypeReflectionCreationHelperImpl<std::tuple<Others...>>::create(tag, detail_name, param);
    }
};

}

namespace _impl {

template<CoreTypeTag tag>
void TypeReflectionRegistrationHelperImpl::register_creator(std::string_view name, CoreTypeCreator<tag> ctor) noexcept {
    auto &m = TypeReflectionManager::instance();
    constexpr auto index = index_of_core_type_tag<tag>;
    std::get<index>(m._creators).emplace(name, std::move(ctor));
    std::get<index>(m._derived_classes).emplace(name, m._classes.back());
}

}

template<CoreTypeTag tag>
struct WrapBaseTag {
    static constexpr auto base_tag = tag;
};

}

#define implements ,

#define DERIVED_CLASS(Cls, Parent)                                                                                \
    class Cls;                                                                                                    \
    namespace _impl {                                                                                             \
        template<> struct TypeReflectionInfoImpl<Cls> {                                                           \
            static constexpr std::string_view name = #Cls;                                                        \
            static constexpr auto is_core = is_core_type<Cls>;                                                    \
            static_assert(is_core || name == name_of_core_type<Cls>, "Inconsistent core type name.");             \
            static constexpr auto base_tag = is_core ? tag_of_core_type<Cls> : base_tag_of_derived_type<Parent>;  \
            static constexpr std::string_view parent_name = TypeReflectionInfoImpl<Parent>::name;                 \
            TypeReflectionInfoImpl() noexcept {                                                                   \
                TypeReflectionRegistrationHelperImpl::register_class(name, parent_name);                          \
            }                                                                                                     \
        };                                                                                                        \
        inline TypeReflectionInfoImpl<Cls> _refl_##Cls{};                                                         \
    }                                                                                                             \
    class Cls                                                                                                     \
        : public virtual Parent,                                                                                  \
          public virtual WrapBaseTag<_impl::TypeReflectionInfoImpl<Cls>::base_tag>

#define CORE_CLASS(Cls)  \
    DERIVED_CLASS(Cls, Empty)

#define PROPERTY(Type, name, tag)                                                                           \
        static_assert(true);                                                                                \
    private:                                                                                                \
        inline static struct _refl_##name##_helper {                                                        \
            _refl_##name##_helper() noexcept {                                                              \
                _impl::TypeReflectionRegistrationHelperImpl::register_property(#name, tag);                 \
            }                                                                                               \
        } _refl_##name{};                                                                                   \
    protected:                                                                                              \
        Type _##name{};                                                                                     \
    public:                                                                                                 \
        bool decode_##name(const CoreTypeCreatorParameterSet &param_set) {                                  \
            if (auto iter = param_set.find(#name); iter != param_set.end()) {                               \
                _decode_##name##_impl(std::get<std::vector<TypeOfCoreTypeTag<tag>>>(param_set.at(#name)));  \
                return true;                                                                                \
            }                                                                                               \
            return false;                                                                                   \
        }                                                                                                   \
    private:                                                                                                \
        virtual void _decode_##name##_impl(const std::vector<TypeOfCoreTypeTag<tag>> &params)

#define CREATOR(detail_name)                                                                                             \
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
        [[nodiscard]] static TypeOfCoreTypeTag<tag> create(const CoreTypeCreatorParameterSet &param_set)
        