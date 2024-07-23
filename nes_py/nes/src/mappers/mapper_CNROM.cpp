//  Program:      nes-py
//  File:         mapper_CNROM.cpp
//  Description:  An implementation of the CNROM mapper
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#include "mappers/mapper_CNROM.hpp"
#include "log.hpp"

namespace NES {

void MapperCNROM::writeCHR(NES_Address address, NES_Byte value) {
    LOG(Info) <<
        "Read-only CHR memory write attempt at " <<
        std::hex <<
        address <<
        std::endl;
}
/// Serializable

void MapperCNROM::serialize(std::vector<uint8_t>& buffer) {
    serialize_bool(is_one_bank, buffer);
    serialize_int(select_chr, buffer);
}

std::span<uint8_t> MapperCNROM::deserialize(std::span<uint8_t> buffer) {
    deserialize_bool(buffer, is_one_bank);
    deserialize_int(buffer, select_chr);
    return buffer;
}
}  // namespace NES
