//
// Created by Mike Smith on 2019/10/3.
//

#pragma once

#include <memory>
#include <type_traits>
#include <string_view>
#include <exception>

#include <util/exception.h>
#include <util/noncopyable.h>
#include <util/string_manipulation.h>

namespace luisa {

LUISA_MAKE_ERROR_TYPE(CoreTypeError);

#define THROW_CORE_TYPE_ERROR(...)  \
    LUISA_THROW_ERROR(CoreTypeError, __VA_ARGS__)

enum struct CoreTypeTag : uint32_t {
    
    // non-value types, indexed from zero
        CAMERA = 0,
    SAMPLER = 1,
    INTEGRATOR = 2,
    TRANSFORM = 3,
    MATERIAL = 4,
    SHAPE = 5,
    LIGHT = 6,
    FILM = 7,
    FILTER = 8,
    SAVER = 9,
    TASK = 10,
    
    // only for counting
        NON_VALUE_TYPE_COUNT,
    
    // value types have no reflection, so no indices are explicitly assigned
        STRING, BOOL, FLOAT, INTEGER,
    
    // for counting
        ALL_TYPE_COUNT
};

constexpr auto non_value_core_type_count = static_cast<size_t>(CoreTypeTag::NON_VALUE_TYPE_COUNT);
constexpr auto all_core_type_count = static_cast<size_t>(CoreTypeTag::ALL_TYPE_COUNT);

template<CoreTypeTag tag>
struct InfoOfCoreTypeTag {};

// value-types
template<>
struct InfoOfCoreTypeTag<CoreTypeTag::STRING> {
    using Type = std::string;
    static constexpr std::string_view name = "String";
    static constexpr auto is_value_type = true;
};

template<>
struct InfoOfCoreTypeTag<CoreTypeTag::BOOL> {
    using Type = bool;
    static constexpr std::string_view name = "Bool";
    static constexpr auto is_value_type = true;
};

template<>
struct InfoOfCoreTypeTag<CoreTypeTag::FLOAT> {
    using Type = float;
    static constexpr std::string_view name = "Float";
    static constexpr auto is_value_type = true;
};

template<>
struct InfoOfCoreTypeTag<CoreTypeTag::INTEGER> {
    using Type = int32_t;
    static constexpr std::string_view name = "Integer";
    static constexpr auto is_value_type = true;
};

#define MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(tag, Cls)                                                                               \
    class Cls;                                                                                                                        \
    template<>                                                                                                                        \
    struct InfoOfCoreTypeTag<tag> {                                                                                                   \
        using Type = std::shared_ptr<Cls>;                                                                                            \
        static constexpr std::string_view name = #Cls;                                                                                \
        static constexpr auto is_value_type = static_cast<uint32_t>(tag) > non_value_core_type_count;  \
    }

// non-value types
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::CAMERA, Camera);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::SAMPLER, Sampler);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::INTEGRATOR, Integrator);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::TRANSFORM, Transform);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::MATERIAL, Material);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::SHAPE, Shape);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::LIGHT, Light);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::FILM, Film);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::FILTER, Filter);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::SAVER, Saver);
MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG(CoreTypeTag::TASK, Task);

#undef MAKE_INFO_FOR_NON_VALUE_CORE_TYPE_TAG

template<CoreTypeTag tag>
using TypeOfCoreTypeTag = typename InfoOfCoreTypeTag<tag>::Type;

namespace _impl {

template<typename List, CoreTypeTag tag = static_cast<CoreTypeTag>(0u)>
struct CoreTypeVectorVariantImpl {};

template<typename ...T, CoreTypeTag tag>
struct CoreTypeVectorVariantImpl<std::variant<T...>, tag> {
    static constexpr auto next_tag = static_cast<CoreTypeTag>(static_cast<uint32_t>(tag) + 1);
    using Type = typename CoreTypeVectorVariantImpl<std::variant<T..., std::vector<TypeOfCoreTypeTag<tag>>>, next_tag>::Type;
};

template<typename ...T>
struct CoreTypeVectorVariantImpl<std::variant<T...>, CoreTypeTag::NON_VALUE_TYPE_COUNT> {
    static constexpr auto next_tag = static_cast<CoreTypeTag>(non_value_core_type_count + 1);
    using Type = typename CoreTypeVectorVariantImpl<std::variant<T...>, next_tag>::Type;
};

template<typename ...T>
struct CoreTypeVectorVariantImpl<std::variant<T...>, CoreTypeTag::ALL_TYPE_COUNT> {
    using Type = std::variant<T...>;
};

}

using CoreTypeVectorVariant = typename _impl::CoreTypeVectorVariantImpl<std::variant<std::monostate>>::Type;
using CoreTypeInitializerParameterSet = std::unordered_map<std::string_view, CoreTypeVectorVariant>;

struct CoreTypeBase : util::Noncopyable {
    virtual ~CoreTypeBase() noexcept = default;
    virtual void initialize(Device &device, const CoreTypeInitializerParameterSet &param_set) = 0;
};

namespace _impl {

template<typename T, CoreTypeTag tag = static_cast<CoreTypeTag>(0u)>
struct TagOfNonValueCoreTypeImpl {
    static constexpr auto value = [] {
        if constexpr (std::is_same_v<T, typename TypeOfCoreTypeTag<tag>::element_type>) {
            return tag;
        } else {
            return TagOfNonValueCoreTypeImpl<T, static_cast<CoreTypeTag>(static_cast<uint32_t>(tag) + 1u)>::value;
        }
    }();
};

template<typename T>
struct TagOfNonValueCoreTypeImpl<T, CoreTypeTag::NON_VALUE_TYPE_COUNT> {
    static constexpr auto value = CoreTypeTag::NON_VALUE_TYPE_COUNT;
};

template<size_t ...tags>
[[nodiscard]] constexpr std::string_view name_of_non_value_core_type_tag(CoreTypeTag tag, std::index_sequence<tags...>) noexcept {
    constexpr std::string_view names[]{InfoOfCoreTypeTag<static_cast<CoreTypeTag>(tags)>::name...};
    return static_cast<uint32_t>(tag) < non_value_core_type_count ? names[static_cast<uint32_t>(tag)] : "Unknown";
}

}

template<typename T>
constexpr auto tag_of_non_value_core_type = _impl::TagOfNonValueCoreTypeImpl<T>::value;

[[nodiscard]] constexpr std::string_view name_of_non_value_core_type_tag(CoreTypeTag tag) noexcept {
    return _impl::name_of_non_value_core_type_tag(tag, std::make_index_sequence<non_value_core_type_count>{});
}

[[nodiscard]] constexpr std::string_view name_of_value_core_type_tag(CoreTypeTag tag) noexcept {
    switch (tag) {
        case CoreTypeTag::STRING:
            return InfoOfCoreTypeTag<CoreTypeTag::STRING>::name;
        case CoreTypeTag::BOOL:
            return InfoOfCoreTypeTag<CoreTypeTag::BOOL>::name;
        case CoreTypeTag::FLOAT:
            return InfoOfCoreTypeTag<CoreTypeTag::FLOAT>::name;
        case CoreTypeTag::INTEGER:
            return InfoOfCoreTypeTag<CoreTypeTag::INTEGER>::name;
        default:
            return "Unknown";
    }
}

[[nodiscard]] constexpr std::string_view name_of_core_type_tag(CoreTypeTag tag) noexcept {
    auto index = static_cast<uint32_t>(tag);
    if (index < non_value_core_type_count) {
        return name_of_non_value_core_type_tag(tag);
    } else if (index > non_value_core_type_count && index < all_core_type_count) {
        return name_of_value_core_type_tag(tag);
    }
    return "Unknown";
}

}
