//
// Created by Mike Smith on 2019/11/1.
//

#include "compatibility.h"
#include <samplers/halton_sampler.h>

using namespace luisa;

template<uint32_t N>
inline unsigned int tea(uint32_t v0, uint32_t v1) {
    auto s0 = 0u;
    for (auto n = 0u; n < N; n++) {
        s0 += 0x9e3779b9u;
        v0 += ((v1 << 4u) + 0xa341316cu) ^ (v1 + s0) ^ ((v1 >> 5u) + 0xc8013ea4u);
        v1 += ((v0 << 4u) + 0xad90777du) ^ (v0 + s0) ^ ((v0 >> 5u) + 0x7e95761eu);
    }
    return v0;
}

constexpr unsigned int primes[256] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
    227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311,
    313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503,
    509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613,
    617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719,
    727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827,
    829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049,
    1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283,
    1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
    1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
    1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619};

inline float halton(thread HaltonSamplerState &state) {
    auto b = primes[state.dimension];
    auto f = 1.0f;
    auto inv_b = 1.0f / b;
    auto r = 0.0f;
    for (auto i = state.offset; i != 0u; i /= b) {
        f = f * inv_b;
        r = r + f * static_cast<float>(i % b);
    }
    state.dimension++;
    return math::clamp(r, 0.0f, 1.0f);
}

using namespace math;
using namespace metal;

kernel void halton_sampler_prepare_for_frame(
    constant HaltonSamplerPrepareForFrameUniforms &uniforms [[buffer(0)]],
    device HaltonSamplerState *states [[buffer(1)]],
    uint2 tid [[thread_position_in_grid]]) {
    
    if (tid.x < uniforms.frame_size.x && tid.y < uniforms.frame_size.y) {
        auto index = tid.y * uniforms.frame_size.x + tid.x;
        auto offset = tea<4>(tid.x, tid.y) + uniforms.frame_index;
        states[index] = {offset, 0};
    }
    
}

kernel void halton_sampler_generate_samples(
    constant HaltonSamplerGenerateSamplesUniforms &uniforms [[buffer(0)]],
    device HaltonSamplerState *states [[buffer(1)]],
    texture2d<float, access::write> random [[texture(0)]],
    uint2 tid [[thread_position_in_grid]]) {
    
    if (tid.x < uniforms.frame_size.x && tid.y < uniforms.frame_size.y) {
        auto index = tid.y * uniforms.frame_size.x + tid.x;
        auto state = states[index];
        float4 v{};
        switch (uniforms.dimensions) {
            case 4:
                v.a = halton(state);
                [[fallthrough]];
            case 3:
                v.b = halton(state);
                [[fallthrough]];
            case 2:
                v.g = halton(state);
                [[fallthrough]];
            case 1:
                v.r = halton(state);
                break;
            default:
                break;
        }
        random.write(v, tid);
    }
}
