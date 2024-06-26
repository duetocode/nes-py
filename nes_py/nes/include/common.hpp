//  Program:      nes-py
//  File:         common.hpp
//  Description:  This file defines common types used in the project
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#ifndef COMMON_HPP
#define COMMON_HPP

// resolve an issue with MSVC overflow during compilation (Windows)
#define _CRT_DECLARE_NONSTDC_NAMES 0
#include <cstdint>
#include <vector>
#include <span>
#include <cstdint>
#include <type_traits>

namespace NES {

/// A shortcut for a byte
typedef uint8_t NES_Byte;
/// A shortcut for a memory address (16-bit)
typedef uint16_t NES_Address;
/// A shortcut for a single pixel in memory
typedef uint32_t NES_Pixel;

class Serializable {
public:
    virtual void serialize(std::vector<uint8_t>& buffer) = 0;
    virtual std::span<uint8_t> deserialize(std::span<uint8_t> buffer) = 0;
protected:
    static void serialize_vector(const std::vector<uint8_t>& value, std::vector<uint8_t>& buffer);
    static std::span<uint8_t> deserialize_vector(std::span<uint8_t> buffer, std::vector<uint8_t>& value);
    static void serialize_bool(bool value, std::vector<uint8_t>& buffer); 
    static void deserialize_bool(std::span<uint8_t>& buffer, bool& value);
};

template<typename T>
inline void serialize_int(const T value, std::vector<uint8_t>& buffer) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    using UnsignedT = std::make_unsigned_t<T>;
    UnsignedT unsigned_value = static_cast<UnsignedT>(value);
    for (size_t i = 0; i < sizeof(UnsignedT); i++) {
        buffer.push_back((unsigned_value >> (i * 8)) & 0xFF);
    }
}

template<typename T>
inline void deserialize_int(std::span<uint8_t>& buffer, T& value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    using UnsignedT = std::make_unsigned_t<T>;
    UnsignedT unsigned_value = 0;
    for (size_t i = 0; i < sizeof(UnsignedT); i++) {
        unsigned_value |= static_cast<UnsignedT>(buffer[0]) << (i * 8);
        buffer = buffer.subspan(1);
    }
    value = static_cast<T>(unsigned_value);
}  // namespace NES

template<typename T>
void serialize_enum(T value, std::vector<uint8_t>& buffer) {
    static_assert(std::is_enum<T>::value, "T must be an enum type");
    serialize_int(static_cast<std::underlying_type_t<T>>(value), buffer);
}

template<typename T>
void deserialize_enum(std::span<uint8_t>& buffer, T& value) {
    static_assert(std::is_enum<T>::value, "T must be an enum type");
    std::underlying_type_t<T> underlying_value;
    deserialize_int(buffer, underlying_value);
    value = static_cast<T>(underlying_value);
}

}

#endif  // COMMON_HPP
