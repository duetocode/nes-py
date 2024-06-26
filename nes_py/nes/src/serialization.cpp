//  Program:      nes-py
//  File:         serialization.cpp
//  Description:  This class implements basic serialization utility functions
//
//  Copyright (c) 2024 Zhao Liang. All rights reserved.
//

#include "common.hpp"

namespace NES {

void Serializable::serialize_vector(const std::vector<uint8_t>& value, std::vector<uint8_t>& buffer) {
    serialize_int(value.size(), buffer);
    buffer.insert(buffer.end(), value.begin(), value.end());
}

std::span<uint8_t> Serializable::deserialize_vector(std::span<uint8_t> buffer, std::vector<uint8_t>& value) {
    // read the length
    size_t size = 0;
    deserialize_int(buffer, size);
    // read the data
    value.clear();
    value.resize(size);
    value.insert(value.end(), buffer.begin(), buffer.begin() + size);
    return buffer.subspan(size);
}

// Specialization for bool serialization
void Serializable::serialize_bool(bool value, std::vector<uint8_t>& buffer) {
    buffer.push_back(value ? 1 : 0);
}

// Specialization for bool deserialization
void Serializable::deserialize_bool(std::span<uint8_t>& buffer, bool& value) {
    if (!buffer.empty()) {
        value = buffer[0] != 0;
        buffer = buffer.subspan(1); // Move the span forward
    }
}

}