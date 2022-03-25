#ifndef DUNE_ENDIAN_H
#define DUNE_ENDIAN_H

#include <cstdint>

namespace dune {

inline uint16_t read_be_uint16(const uint8_t* p) {
    return static_cast<uint16_t>((p[0] << 8u) | p[1]);
}

inline uint16_t read_be_uint16(const uint16_t& unaligned_word) {
    const auto* p = reinterpret_cast<const uint8_t*>(&unaligned_word);
    return read_be_uint16(p);
}

inline uint32_t read_be_uint32(const uint8_t* p) {
    return (p[0] << 24u) | (p[1] << 16u) | (p[2] << 8u) | p[3];
}

inline uint32_t read_be_uint32(const uint32_t& unaligned_word) {
    const auto* p = reinterpret_cast<const uint8_t*>(&unaligned_word);
    return read_be_uint32(p);
}

inline uint16_t read_le_uint16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8u));
}

inline uint16_t read_le_uint16(const uint16_t& unaligned_word) {
    const auto* p = reinterpret_cast<const uint8_t*>(&unaligned_word);
    return read_le_uint16(p);
}

inline int read_le_uint32(const uint8_t* p) {
    return (p[0]) | (p[1] << 8u) | (p[2] << 16u) | (p[3] << 24);
}

inline uint32_t read_le_uint32(const uint32_t& unaligned_word) {
    const auto* p = reinterpret_cast<const uint8_t*>(&unaligned_word);
    return read_le_uint32(p);
}

} // namespace dune

#endif // DUNE_ENDIAN_H
