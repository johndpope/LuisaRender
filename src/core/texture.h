//
// Created by Mike Smith on 2019/10/24.
//

#pragma once

#include <util/noncopyable.h>

#include "mathematics.h"
#include "buffer.h"

namespace luisa {

enum struct TextureFormatTag {
    RGBA32F,
    GRAYSCALE32F
};

enum struct TextureAccessTag {
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
};

class Texture : util::Noncopyable {

protected:
    math::uint2 _size;
    TextureFormatTag _format;
    TextureAccessTag _access;

public:
    Texture(math::uint2 size, TextureFormatTag format_tag, TextureAccessTag access_tag) noexcept
        : _size{size}, _format{format_tag}, _access{access_tag} {}
    virtual ~Texture() noexcept = default;
    virtual void copy_from_buffer(struct KernelDispatcher &dispatch, Buffer &buffer) = 0;
    virtual void copy_to_buffer(struct KernelDispatcher &dispatch, Buffer &buffer) = 0;
    
    [[nodiscard]] size_t bytes_per_pixel() const noexcept {
        switch (_format) {
            case TextureFormatTag::RGBA32F:
                return sizeof(math::float4);
            case TextureFormatTag::GRAYSCALE32F:
                return sizeof(float);
        }
    }
    
    [[nodiscard]] size_t bytes_per_row() const noexcept {
        return bytes_per_pixel() * _size.x;
    }
    
    [[nodiscard]] size_t bytes_per_image() const noexcept {
        return bytes_per_row() * _size.y;
    }
    
    [[nodiscard]] math::uint2 size() const noexcept {
        return _size;
    }
    
};

}