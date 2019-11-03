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

#include "core_type.h"

namespace luisa {

struct TypeReflectionError : public std::runtime_error {
    template<typename ...Args>
    TypeReflectionError(std::string_view file, size_t line, Args &&...args) noexcept
        : std::runtime_error{util::serialize("TypeReflectionError: ", std::forward<Args>(args)..., "  [file: \"", file, "\", line: ", line, "]")} {}
};

#define THROW_TYPE_REFLECTION_ERROR(...)  throw TypeReflectionError{__FILE__, __LINE__, __VA_ARGS__}

using CoreTypeDecoderParameterSet = std::unordered_map<std::string_view, CoreTypeVectorVariant>;

struct CoreTypeBase : util::Noncopyable {  // convenience base type for core types
    virtual ~CoreTypeBase() noexcept = default;
    virtual void decode(const CoreTypeDecoderParameterSet &param_set [[maybe_unused]]) = 0;
};

template<CoreTypeTag tag>
using CoreTypeCreator = std::function<TypeOfCoreTypeTag<tag>()>;

template<CoreTypeTag tag>
using CoreTypeDecodingCreator = std::function<TypeOfCoreTypeTag<tag>(const CoreTypeDecoderParameterSet &)>;

namespace _impl {

template<typename T>
struct TypeReflectionInfoImpl {};

template<>
struct TypeReflectionInfoImpl<CoreTypeBase> {
    static constexpr std::string_view name{};
};

struct TypeReflectionRegistrationHelperImpl {
    
    static void register_class(std::string_view cls, std::string_view parent) noexcept;
    static void register_property(std::string_view prop, CoreTypeTag tag) noexcept;
    
    template<CoreTypeTag tag>
    static void register_creator(std::string_view name, CoreTypeCreator<tag> ctor) noexcept;
    
    template<CoreTypeTag tag>
    static void register_decoding_creator(std::string_view name, CoreTypeDecodingCreator<tag> ctor) noexcept;
    
};

}

namespace _impl {

template<CoreTypeTag tag>
using MapCoreTypeTagToCreatorListImpl = std::unordered_map<std::string_view, CoreTypeCreator<tag>>;

template<CoreTypeTag tag>
using MapCoreTypeTagToDecodingCreatorListImpl = std::unordered_map<std::string_view, CoreTypeDecodingCreator<tag>>;

template<CoreTypeTag tag>
using MapCoreTypeTagToDerivedTypeClassNameListImpl = std::unordered_map<std::string_view, std::string_view>;

}

using CoreTypeCreatorList = MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToCreatorListImpl>;
using CoreTypeDecodingCreatorList = MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToDecodingCreatorListImpl>;
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
    CoreTypeDecodingCreatorList _decoding_creators;
    DerivedTypeClassNameList _derived_classes;
    
    friend _impl::TypeReflectionRegistrationHelperImpl;
    TypeReflectionManager() = default;
    
    template<CoreTypeTag first_tag, CoreTypeTag ...other_tags>
    [[nodiscard]] std::string_view _derived_class_name_impl(
        CoreTypeTag tag, std::string_view detail_name,
        std::tuple<WrapCoreTypeTag<first_tag>, WrapCoreTypeTag<other_tags>...>) const {
        if (tag == first_tag) {
            return derived_class_name<first_tag>(detail_name);
        }
        if constexpr (sizeof...(other_tags) != 0) {
            return _derived_class_name_impl(tag, detail_name, std::tuple<WrapCoreTypeTag<other_tags>...>{});
        }
        THROW_TYPE_REFLECTION_ERROR("unknown tag.");
    }

public:
    TypeReflectionManager(TypeReflectionManager &&) = delete;
    TypeReflectionManager(const TypeReflectionManager &) = delete;
    TypeReflectionManager &operator=(TypeReflectionManager &&) = delete;
    TypeReflectionManager &operator=(const TypeReflectionManager &) = delete;

public:
    [[nodiscard]] static TypeReflectionManager &instance() noexcept;
    [[nodiscard]] const PropertyList &properties(std::string_view cls) const;
    [[nodiscard]] std::string_view parent(std::string_view cls) const;
    [[nodiscard]] const std::vector<std::string_view> &classes() const noexcept;
    [[nodiscard]] bool is_core(std::string_view cls) const;
    [[nodiscard]] CoreTypeTag property_tag(std::string_view cls, std::string_view prop) const;
    [[nodiscard]] static CoreTypeVariant create(CoreTypeTag tag, std::string_view detail_name);
    [[nodiscard]] static CoreTypeVariant create(std::string_view base_type, std::string_view detail_name);
    [[nodiscard]] static CoreTypeVariant create_and_decode(CoreTypeTag tag, std::string_view detail_name, const CoreTypeDecoderParameterSet &params);
    [[nodiscard]] static CoreTypeVariant create_and_decode(std::string_view base_type, std::string_view detail_name, const CoreTypeDecoderParameterSet &params);
    [[nodiscard]] std::string_view derived_class_name(CoreTypeTag tag, std::string_view detail_name) const;
    
    template<typename T>
    [[nodiscard]] static constexpr std::string_view name() noexcept {
        return _impl::TypeReflectionInfoImpl<T>::name;
    }
    
    template<typename T>
    [[nodiscard]] static constexpr std::string_view parent() noexcept {
        return _impl::TypeReflectionInfoImpl<T>::parent_name;
    }
    
    template<typename T>
    [[nodiscard]] const PropertyList &properties() const {
        return properties(name<T>());
    }
    
    template<typename T>
    [[nodiscard]] CoreTypeTag property_tag(std::string_view prop) const {
        return property_tag(name<T>(), prop);
    }
    
    template<CoreTypeTag tag>
    [[nodiscard]] auto create(std::string_view detail_name) {
        auto &&ctors = std::get<index_of_core_type_tag<tag>>(_creators);
        auto iter = ctors.find(detail_name);
        if (iter == ctors.end()) {
            THROW_TYPE_REFLECTION_ERROR("creator not registered for class ", name_of_core_type_tag(tag), "::", detail_name, ".");
        }
        return iter->second();
    }
    
    template<CoreTypeTag tag>
    [[nodiscard]] auto create_and_decode(std::string_view detail_name, const CoreTypeDecoderParameterSet &params) {
        auto &&ctors = std::get<index_of_core_type_tag<tag>>(_creators);
        auto iter = ctors.find(detail_name);
        if (iter == ctors.end()) {
            THROW_TYPE_REFLECTION_ERROR("creator not registered for class ", name_of_core_type_tag(tag), "::", detail_name, ".");
        }
        auto instance = iter->second();
        std::dynamic_pointer_cast<CoreTypeBase>(instance)->decode(params);
        return instance;
    }
    
    template<CoreTypeTag tag>
    [[nodiscard]] std::string_view derived_class_name(std::string_view detail_name) const {
        auto &&class_list = std::get<index_of_core_type_tag<tag>>(_derived_classes);
        auto iter = class_list.find(detail_name);
        if (iter == class_list.end()) {
            THROW_TYPE_REFLECTION_ERROR("unregistered derived class ", name_of_core_type_tag(tag), "::", detail_name, ".");
        }
        return iter->second;
    }
};

namespace _impl {

template<typename Tuple>
struct TypeReflectionCreationHelperImpl {
    static CoreTypeVariant create(CoreTypeTag, std::string_view detail_name) {
        THROW_TYPE_REFLECTION_ERROR("failed to create instance for detail type \"", detail_name, "\".");
    }
    static CoreTypeVariant create_and_decode(CoreTypeTag, std::string_view detail_name, const CoreTypeDecoderParameterSet &) {
        THROW_TYPE_REFLECTION_ERROR("failed to create instance for detail type \"", detail_name, "\".");
    }
};

template<typename First, typename ...Others>
struct TypeReflectionCreationHelperImpl<std::tuple<First, Others...>> {
    static auto create(CoreTypeTag tag, std::string_view detail_name) -> CoreTypeVariant {
        if constexpr (!is_core_type_tag_value_type<First::tag>) {
            return tag == First::tag ?
                   TypeReflectionManager::instance().create<First::tag>(detail_name) :
                   TypeReflectionCreationHelperImpl<std::tuple<Others...>>::create(tag, detail_name);
        } else {
            return TypeReflectionCreationHelperImpl<std::tuple<Others...>>::create(tag, detail_name);
        }
    }
    static auto create_and_decode(CoreTypeTag tag, std::string_view detail_name, const CoreTypeDecoderParameterSet &params) -> CoreTypeVariant {
        if constexpr (!is_core_type_tag_value_type<First::tag>) {
            return tag == First::tag ?
                   TypeReflectionManager::instance().create_and_decode<First::tag>(detail_name, params) :
                   TypeReflectionCreationHelperImpl<std::tuple<Others...>>::create_and_decode(tag, detail_name, params);
        } else {
            return TypeReflectionCreationHelperImpl<std::tuple<Others...>>::create_and_decode(tag, detail_name, params);
        }
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
            static_assert(!is_core || name == name_of_core_type<Cls>, "Inconsistent core type name: "#Cls);       \
            static constexpr auto base_tag = is_core ? tag_of_core_type<Cls> : base_tag_of_derived_type<Parent>;  \
            static constexpr std::string_view parent_name = TypeReflectionInfoImpl<Parent>::name;                 \
            static auto do_register() noexcept {                                                                  \
                TypeReflectionRegistrationHelperImpl::register_class(name, parent_name);                          \
                return 0;                                                                                         \
            }                                                                                                     \
        };                                                                                                        \
        inline auto _refl_##Cls = TypeReflectionInfoImpl<Cls>::do_register();                                     \
    }                                                                                                             \
    class Cls                                                                                                     \
        : public virtual Parent,                                                                                  \
          public virtual WrapBaseTag<_impl::TypeReflectionInfoImpl<Cls>::base_tag>

#define CORE_CLASS(Cls)  \
    DERIVED_CLASS(Cls, CoreTypeBase)

#define PROPERTY(Type, name, tag)                                                                           \
        static_assert(true);                                                                                \
    private:                                                                                                \
        inline static struct _refl_##name##_helper {                                                        \
            _refl_##name##_helper() noexcept {                                                              \
                ::luisa::_impl::TypeReflectionRegistrationHelperImpl::register_property(#name, tag);        \
            }                                                                                               \
        } _refl_##name{};                                                                                   \
    protected:                                                                                              \
        Type _##name{};                                                                                     \
        bool _decode_##name(const CoreTypeDecoderParameterSet &param_set) {                                 \
            if (auto iter = param_set.find(#name); iter != param_set.end()) {                               \
                _decode_##name##_impl(std::get<std::vector<TypeOfCoreTypeTag<tag>>>(param_set.at(#name)));  \
                return true;                                                                                \
            }                                                                                               \
            return false;                                                                                   \
        }                                                                                                   \
    private:                                                                                                \
        virtual void _decode_##name##_impl(const std::vector<TypeOfCoreTypeTag<tag>> &params)

#define CREATOR(detail_name)                                                                                                                                          \
        static_assert(true);                                                                                                                                          \
    private:                                                                                                                                                          \
        inline static struct _refl_ctor_helper {                                                                                                                      \
            _refl_ctor_helper() noexcept {                                                                                                                            \
                ::luisa::_impl::TypeReflectionRegistrationHelperImpl::register_creator<base_tag>(detail_name, create);                                                \
                ::luisa::_impl::TypeReflectionRegistrationHelperImpl::register_decoding_creator<base_tag>(detail_name, [&] (const CoreTypeDecoderParameterSet &ps) {  \
                    auto instance = create();                                                                                                                         \
                    instance->decode(ps);                                                                                                                             \
                    return instance;                                                                                                                                  \
                });                                                                                                                                                   \
            }                                                                                                                                                         \
        } _refl_ctor{};                                                                                                                                               \
    public:                                                                                                                                                           \
        [[nodiscard]] static TypeOfCoreTypeTag<base_tag> create()
        