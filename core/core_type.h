//
// Created by Mike Smith on 2019/10/3.
//

#pragma once

#include <memory>
#include <type_traits>
#include <string_view>

namespace luisa {

enum struct CoreTypeTag : uint32_t {
    UNKNOWN,
    STRING, BOOL, FLOAT, INTEGER,
    CAMERA, SAMPLER, INTEGRATOR, TRANSFORM, MATERIAL, SHAPE, LIGHT, FILM, FILTER, SAVER, TASK
};

constexpr std::string_view name_of_core_type_tag(CoreTypeTag t) noexcept {
    switch (t) {
        case CoreTypeTag::STRING:
            return "String";
        case CoreTypeTag::BOOL:
            return "Bool";
        case CoreTypeTag::FLOAT:
            return "Float";
        case CoreTypeTag::INTEGER:
            return "Integer";
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

using AllCoreTypeInfo = std::tuple<
    CoreTypeInfo<std::string, CoreTypeTag::STRING, true>,
    CoreTypeInfo<bool, CoreTypeTag::BOOL, true>,
    CoreTypeInfo<float, CoreTypeTag::FLOAT, true>,
    CoreTypeInfo<int32_t, CoreTypeTag::INTEGER, true>,
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
    CoreTypeInfo<class Task, CoreTypeTag::TASK, false>>;

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

constexpr auto core_type_count = _impl::CoreTypeCountImpl<AllCoreTypeInfo>::value;

template<CoreTypeTag tag>
constexpr auto index_of_core_type_tag = _impl::IndexOfCoreTypeTagImpl<AllCoreTypeInfo, tag>::value;

template<CoreTypeTag tag>
using InfoOfCoreTypeTag = typename _impl::InfoOfCoreTypeTagImpl<AllCoreTypeInfo, tag>::Type;

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
constexpr auto tag_of_core_type = _impl::TagOfCoreTypeImpl<AllCoreTypeInfo, type>::value;

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
constexpr auto base_tag_of_derived_type = _impl::BaseTagOfDerivedType<AllCoreTypeInfo, T>::value;

}
