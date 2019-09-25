//
// Created by Mike Smith on 2019/9/25.
//

#pragma once

#import <simd/simd.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>

namespace luisa {

namespace impl {

template<typename T, size_t n>
struct SelectVectorTypeImpl {};

template<>
struct SelectVectorTypeImpl<float, 2> {
    using Type = simd::float2;
};

template<>
struct SelectVectorTypeImpl<float, 3> {
    using Type = simd::float3;
};

template<>
struct SelectVectorTypeImpl<int, 2> {
    using Type = simd::int2;
};

template<>
struct SelectVectorTypeImpl<int, 3> {
    using Type = simd::int3;
};

template<typename T, size_t n>
using SelectVectorType = typename SelectVectorTypeImpl<T, n>::Type;

}

template<typename T, size_t n>
using Vector = impl::SelectVectorType<T, n>;

template<typename T>
using Vector2 = Vector<T, 2>;

template<typename T>
using Vector3 = Vector<T, 3>;

using Vector2i = Vector2<int>;
using Vector3i = Vector3<int>;
using Vector2f = Vector2<float>;
using Vector3f = Vector3<float>;

template<typename T>
using Point2 = Vector2<T>;

template<typename T>
using Point3 = Vector3<T>;

using Point2i = Point2<int>;
using Point3i = Point3<int>;
using Point2f = Point2<float>;
using Point3f = Point3<float>;

template<typename T>
using Normal3 = Vector3<T>;

using Normal3f = Normal3<float>;

template<typename T>
struct Bounds2 {
    
    Point2<T> min;
    Point2<T> max;
    
};

using Bounds2i = Bounds2<int>;
using Bounds2f = Bounds2<float>;

}
