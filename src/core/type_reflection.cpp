//
// Created by Mike Smith on 2019/10/4.
//

#include "type_reflection.h"

namespace luisa {

namespace _impl {

void TypeReflectionRegistrationHelperImpl::register_class(std::string_view cls, std::string_view parent) noexcept {
    
    auto &m = TypeReflectionManager::instance();
    m._classes.emplace_back(cls);
    m._parents[cls] = parent;
    m._curr = &(m._properties[cls] = {});
    
    if (!parent.empty()) {
        for (auto prop : m._properties[parent]) {  // inherit properties from parent
            register_property(prop.first, prop.second);
        }
    }
}

void TypeReflectionRegistrationHelperImpl::register_property(std::string_view prop, CoreTypeTag tag) noexcept {
    auto &m = TypeReflectionManager::instance();
    (*m._curr)[prop] = tag;
}

}

const TypeReflectionManager::PropertyList &TypeReflectionManager::properties(std::string_view cls) const {
    auto iter = _properties.find(cls);
    if (iter == _properties.end()) {
        THROW_TYPE_REFLECTION_ERROR("unregistered class \"", cls, "\".");
    }
    return iter->second;
}

TypeReflectionManager &TypeReflectionManager::instance() noexcept {
    static TypeReflectionManager manager;
    return manager;
}

std::string_view TypeReflectionManager::parent(std::string_view cls) const {
    auto iter = _parents.find(cls);
    if (iter == _parents.end()) {
        THROW_TYPE_REFLECTION_ERROR("unregistered class \"", cls, "\".");
    }
    return iter->second;
}

const std::vector<std::string_view> &TypeReflectionManager::classes() const noexcept {
    return _classes;
}

CoreTypeTag TypeReflectionManager::property_tag(std::string_view cls, std::string_view prop) const {
    auto &&props = properties(cls);
    auto iter = props.find(prop);
    if (iter == props.end()) {
        THROW_TYPE_REFLECTION_ERROR("unknown property \"", prop, "\" in class \"", cls, "\".");
    }
    return iter->second;
}

bool TypeReflectionManager::is_core(std::string_view cls) const {
    auto iter = _parents.find(cls);
    if (iter == _parents.end()) {
        THROW_TYPE_REFLECTION_ERROR("unregistered class \"", cls, "\".");
    }
    return iter->second.empty();
}

CoreTypeVariant TypeReflectionManager::create(CoreTypeTag tag, std::string_view detail_name, const CoreTypeCreatorParameterSet &param) {
    return _impl::TypeReflectionCreationHelperImpl<CoreTypeTagList>::create(tag, detail_name, param);
}

CoreTypeVariant TypeReflectionManager::create(std::string_view base_type, std::string_view detail_name, const CoreTypeCreatorParameterSet &param) {
    return _impl::TypeReflectionCreationHelperImpl<CoreTypeTagList>::create(tag_of_core_type_name(base_type), detail_name, param);
}

std::string_view TypeReflectionManager::derived_class_name(CoreTypeTag tag, std::string_view detail_name) const {
    return _derived_class_name_impl(tag, detail_name, CoreTypeTagList{});
}

}