//
// Created by Mike Smith on 2019/10/3.
//

#pragma once

#include <memory>
#include <type_traits>
#include <string_view>
#include <exception>

#include "string_manipulation.h"

namespace luisa {

struct CoreTypeError : std::runtime_error {
    template<typename ...Args>
    CoreTypeError(std::string_view file, size_t line, Args &&...args) noexcept
        : std::runtime_error{serialize("CoreTypeError: ", std::forward<Args>(args)..., " [ file: \"", file, "\", line: ", line, " ]")} {}
};

#define THROW_CORE_TYPE_ERROR(...) throw CoreTypeError{__FILE__, __LINE__, __VA_ARGS__}

enum struct CoreTypeTag : uint32_t {
    CAMERA, SAMPLER, INTEGRATOR, TRANSFORM, MATERIAL, SHAPE, LIGHT, FILM, FILTER, SAVER, TASK,
    STRING, BOOL, FLOAT, INTEGER,
    UNKNOWN,
};

constexpr std::string_view name_of_core_type_tag(CoreTypeTag t) noexcept {
    switch (t) {
        case CoreTypeTag::CAMERA:
            return "Camera";
        case CoreTypeTag::SAMPLER:
            return "Sampler";
        case CoreTypeTag::INTEGRATOR:
            return "Integrator";
        case CoreTypeTag::TRANSFORM:
            return "Transform";
        case CoreTypeTag::MATERIAL:
            return "Material";
        case CoreTypeTag::SHAPE:
            return "Shape";
        case CoreTypeTag::LIGHT:
            return "Light";
        case CoreTypeTag::FILM:
            return "Film";
        case CoreTypeTag::FILTER:
            return "Filter";
        case CoreTypeTag::SAVER:
            return "Saver";
        case CoreTypeTag::TASK:
            return "Task";
        case CoreTypeTag::STRING:
            return "String";
        case CoreTypeTag::BOOL:
            return "Bool";
        case CoreTypeTag::FLOAT:
            return "Float";
        case CoreTypeTag::INTEGER:
            return "Integer";
        default:
            break;
    }
    return "Unknown";
}

template<typename T, CoreTypeTag t, bool is_value_type>
struct CoreTypeInfo {
    using Type = T;
    static constexpr auto tag = t;
    static constexpr auto is_value = is_value_type;
};

using CoreTypeInfoList = std::tuple<
    CoreTypeInfo<class Camera, CoreTypeTag::CAMERA, false>,
    CoreTypeInfo<class Sampler, CoreTypeTag::SAMPLER, false>,
    CoreTypeInfo<class Integrator, CoreTypeTag::INTEGRATOR, false>,
    CoreTypeInfo<class Transform, CoreTypeTag::TRANSFORM, false>,
    CoreTypeInfo<class Material, CoreTypeTag::MATERIAL, false>,
    CoreTypeInfo<class Shape, CoreTypeTag::SHAPE, false>,
    CoreTypeInfo<class Light, CoreTypeTag::LIGHT, false>,
    CoreTypeInfo<class Film, CoreTypeTag::FILM, false>,
    CoreTypeInfo<class Filter, CoreTypeTag::FILTER, false>,
    CoreTypeInfo<class Saver, CoreTypeTag::SAVER, false>,
    CoreTypeInfo<class Task, CoreTypeTag::TASK, false>,
    CoreTypeInfo<std::string, CoreTypeTag::STRING, true>,
    CoreTypeInfo<bool, CoreTypeTag::BOOL, true>,
    CoreTypeInfo<float, CoreTypeTag::FLOAT, true>,
    CoreTypeInfo<int32_t, CoreTypeTag::INTEGER, true>>;

namespace _impl {

template<typename Tuple>
struct CoreTypeCountImpl {
    static constexpr auto value = 0ul;
};

template<typename ...Info>
struct CoreTypeCountImpl<std::tuple<Info...>> {
    static constexpr auto value = sizeof...(Info);
};

template<typename Tuple, CoreTypeTag tag, int curr_index = 0>
struct IndexOfCoreTypeTagImpl {
    static constexpr auto value = -1;
};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo, CoreTypeTag tag, int index>
struct IndexOfCoreTypeTagImpl<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, tag, index> {
    static constexpr auto value = []() constexpr noexcept {
        if constexpr (first_tag == tag) { return index; }
        return IndexOfCoreTypeTagImpl<std::tuple<OtherInfo...>, tag, index + 1>::value;
    }();
};

template<typename Tuple, CoreTypeTag tag>
struct InfoOfCoreTypeTagImpl {};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo, CoreTypeTag tag>
struct InfoOfCoreTypeTagImpl<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, tag> {
    using Type = typename InfoOfCoreTypeTagImpl<std::tuple<OtherInfo...>, tag>::Type;
};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo>
struct InfoOfCoreTypeTagImpl<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, first_tag> {
    using Type = CoreTypeInfo<FirstType, first_tag, first_is_value_type>;
};

}

constexpr auto core_type_count = _impl::CoreTypeCountImpl<CoreTypeInfoList>::value;

template<CoreTypeTag tag>
constexpr auto index_of_core_type_tag = _impl::IndexOfCoreTypeTagImpl<CoreTypeInfoList, tag>::value;

template<CoreTypeTag tag>
using InfoOfCoreTypeTag = typename _impl::InfoOfCoreTypeTagImpl<CoreTypeInfoList, tag>::Type;

namespace _impl {

template<CoreTypeTag tag, bool is_is_value_type>
struct TypeOfCoreTypeTagImpl {
    using Type = std::shared_ptr<typename InfoOfCoreTypeTag<tag>::Type>;
};

template<CoreTypeTag tag>
struct TypeOfCoreTypeTagImpl<tag, true> {
    using Type = typename InfoOfCoreTypeTag<tag>::Type;
};

}

template<CoreTypeTag tag>
using TypeOfCoreTypeTag = typename _impl::TypeOfCoreTypeTagImpl<tag, InfoOfCoreTypeTag<tag>::is_value>::Type;

namespace _impl {

template<typename Tuple, typename T>
struct TagOfCoreTypeImpl {
    static constexpr auto value = CoreTypeTag::UNKNOWN;
};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo, typename T>
struct TagOfCoreTypeImpl<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, T> {
    static constexpr auto value = []() constexpr noexcept {
        if constexpr (std::is_same_v<FirstType, T>) { return first_tag; }
        return TagOfCoreTypeImpl<std::tuple<OtherInfo...>, T>::value;
    }();
};

}

template<typename type>
constexpr auto tag_of_core_type = _impl::TagOfCoreTypeImpl<CoreTypeInfoList, type>::value;

template<typename type>
constexpr auto name_of_core_type = name_of_core_type_tag(tag_of_core_type<type>);

template<typename type>
constexpr auto is_core_type = (tag_of_core_type<type> != CoreTypeTag::UNKNOWN);

namespace _impl {

template<typename Tuple, typename T>
struct BaseTagOfDerivedType {
    static constexpr auto value = CoreTypeTag::UNKNOWN;
};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo, typename T>
struct BaseTagOfDerivedType<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, T> {
    static constexpr auto value = []() constexpr noexcept {
        if constexpr (std::is_base_of_v<FirstType, T>) { return first_tag; }
        return BaseTagOfDerivedType<std::tuple<OtherInfo...>, T>::value;
    }();
};

}

template<typename T>
constexpr auto base_tag_of_derived_type = _impl::BaseTagOfDerivedType<CoreTypeInfoList, T>::value;

namespace _impl {

template<typename Tuple, typename Result, template<CoreTypeTag> class MapTag>
struct MapCoreTypeInfoListByTagImpl {
    using Type = Result;
};

template<typename FirstType, CoreTypeTag first_tag, bool first_is_value_type, typename ...OtherInfo, typename ...Ctors, template<CoreTypeTag> class Map>
struct MapCoreTypeInfoListByTagImpl<std::tuple<CoreTypeInfo<FirstType, first_tag, first_is_value_type>, OtherInfo...>, std::tuple<Ctors...>, Map> {
    using Type = typename MapCoreTypeInfoListByTagImpl<std::tuple<OtherInfo...>, std::tuple<Ctors..., Map<first_tag>>, Map>::Type;
};

}

template<template<CoreTypeTag> class MapTag>
using MapCoreTypeInfoListByTag = typename _impl::MapCoreTypeInfoListByTagImpl<CoreTypeInfoList, std::tuple<>, MapTag>::Type;

template<CoreTypeTag t>
struct WrapCoreTypeTag {
    static constexpr auto tag = t;
};

namespace _impl {

template<CoreTypeTag tag>
using MapCoreTypeTagToWrapperImpl = WrapCoreTypeTag<tag>;

}

using CoreTypeTagList = MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToWrapperImpl>;
using CoreTypeList = MapCoreTypeInfoListByTag<TypeOfCoreTypeTag>;

namespace _impl {

template<typename Tuple>
struct TagOfCoreTypeNameImpl {
    constexpr static auto get(std::string_view) noexcept {
        return CoreTypeTag::UNKNOWN;
    }
};

template<CoreTypeTag first, CoreTypeTag ...others>
struct TagOfCoreTypeNameImpl<std::tuple<WrapCoreTypeTag<first>, WrapCoreTypeTag<others>...>> {
    constexpr static auto get(std::string_view name) noexcept {
        return name == name_of_core_type_tag(first) ? first : TagOfCoreTypeNameImpl<std::tuple<WrapCoreTypeTag<others>...>>::get(name);
    }
};

}

constexpr CoreTypeTag tag_of_core_type_name(std::string_view name) noexcept {
    return _impl::TagOfCoreTypeNameImpl<CoreTypeTagList>::get(name);
}

namespace _impl {

template<typename Tuple>
struct IndexOfCoreTypeNameImpl {
    constexpr static auto get(std::string_view) noexcept {
        return -1;
    }
};

template<CoreTypeTag first, CoreTypeTag ...others>
struct IndexOfCoreTypeNameImpl<std::tuple<WrapCoreTypeTag<first>, WrapCoreTypeTag<others>...>> {
    constexpr static auto get(std::string_view name) noexcept {
        return name == name_of_core_type_tag(first) ?
               index_of_core_type_tag<first> :
               IndexOfCoreTypeNameImpl<std::tuple<WrapCoreTypeTag<others>...>>::get(name);
    }
};

}

constexpr int index_of_core_type_name(std::string_view name) noexcept {
    return _impl::IndexOfCoreTypeNameImpl<CoreTypeTagList>::get(name);
}

namespace _impl {

template<typename ...T>
constexpr auto is_core_type_name_impl(std::string_view name, std::tuple<T...> list [[maybe_unused]]) noexcept {
    return ((name == name_of_core_type_tag(T::tag)) || ...);
}

}

constexpr auto is_core_type_name(std::string_view name) noexcept {
    return _impl::is_core_type_name_impl(name, CoreTypeTagList{});
}

template<CoreTypeTag tag>
constexpr auto is_core_type_tag_value_type = InfoOfCoreTypeTag<tag>::is_value;

namespace _impl {

template<typename Tuple>
struct VariantFromTupleImpl {};

template<typename ...T>
struct VariantFromTupleImpl<std::tuple<T...>> {
    using Type = std::variant<T...>;
};

template<CoreTypeTag tag>
using MapCoreTypeTagToVectorImpl = std::vector<TypeOfCoreTypeTag<tag>>;

}

template<typename Tuple>
using VariantFromTuple = typename _impl::VariantFromTupleImpl<Tuple>::Type;

using CoreTypeVariant = VariantFromTuple<CoreTypeList>;
using CoreTypeVectorVariant = VariantFromTuple<MapCoreTypeInfoListByTag<_impl::MapCoreTypeTagToVectorImpl>>;

template<CoreTypeTag tag>
inline void core_type_vector_variant_emplace_back(CoreTypeVectorVariant &v, CoreTypeVariant elem) {
    std::get<std::vector<TypeOfCoreTypeTag<tag>>>(v).emplace_back(std::get<TypeOfCoreTypeTag<tag>>(elem));
}

namespace _impl {

template<CoreTypeTag first_tag, CoreTypeTag ...other_tags>
inline void core_type_vector_variant_emplace_back_impl(
    CoreTypeTag tag, CoreTypeVectorVariant &v, CoreTypeVariant elem,
    std::tuple<WrapCoreTypeTag<first_tag>, WrapCoreTypeTag<other_tags>...>) {

    if (tag == first_tag) {
        return core_type_vector_variant_emplace_back<first_tag>(v, std::move(elem));
    }
    if constexpr (sizeof...(other_tags) != 0) {
        return core_type_vector_variant_emplace_back_impl(tag, v, std::move(elem), std::tuple<WrapCoreTypeTag<other_tags>...>{});
    }
    THROW_CORE_TYPE_ERROR("unknown core type tag.");
}

}

inline void core_type_vector_variant_emplace_back(CoreTypeTag tag, CoreTypeVectorVariant &v, CoreTypeVariant elem) {
    return _impl::core_type_vector_variant_emplace_back_impl(tag, v, std::move(elem), CoreTypeTagList{});
}

namespace _impl {

template<CoreTypeTag first_tag, CoreTypeTag ...other_tags>
[[nodiscard]] inline CoreTypeVectorVariant core_type_vector_variant_create_impl(CoreTypeTag tag, CoreTypeVariant elem, std::tuple<WrapCoreTypeTag<first_tag>, WrapCoreTypeTag<other_tags>...>) {
    if (tag == first_tag) {
        return std::vector<TypeOfCoreTypeTag<first_tag>>{std::get<TypeOfCoreTypeTag<first_tag>>(elem)};
    }
    if constexpr (sizeof...(other_tags) != 0) {
        return core_type_vector_variant_create_impl(tag, std::move(elem), std::tuple<WrapCoreTypeTag<other_tags>...>{});
    }
    THROW_CORE_TYPE_ERROR("unknown core type tag.");
}

}

[[nodiscard]] inline CoreTypeVectorVariant core_type_vector_variant_create(CoreTypeTag tag, CoreTypeVariant elem) {
    return _impl::core_type_vector_variant_create_impl(tag, std::move(elem), CoreTypeTagList{});
}

}
