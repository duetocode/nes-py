//  Program:      nes-py
//  File:         main_bus.cpp
//  Description:  This class houses the main bus data for the NES
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#include "main_bus.hpp"
#include "log.hpp"

namespace NES {

NES_Byte MainBus::read(NES_Address address) {
    if (address < 0x2000) {
        return ram[address & 0x7ff];
    } else if (address < 0x4020) {
        if (address < 0x4000) {  // PPU registers, mirrored
            auto reg = static_cast<IORegisters>(address & 0x2007);
            if (read_callbacks.count(reg))
                return read_callbacks.at(reg)();
            else
                LOG(InfoVerbose) << "No read callback registered for I/O register at: " << std::hex << +address << std::endl;
        } else if (address < 0x4018 && address >= 0x4014) {  // only *some* IO registers
            auto reg = static_cast<IORegisters>(address);
            if (read_callbacks.count(reg))
                return read_callbacks.at(reg)();
            else
                LOG(InfoVerbose) << "No read callback registered for I/O register at: " << std::hex << +address << std::endl;
        }
        else {
            LOG(InfoVerbose) << "Read access attempt at: " << std::hex << +address << std::endl;
        }
    } else if (address < 0x6000) {
        LOG(InfoVerbose) << "Expansion ROM read attempted. This is currently unsupported" << std::endl;
    } else if (address < 0x8000) {
        if (mapper->hasExtendedRAM())
            return extended_ram[address - 0x6000];
    } else {
        return mapper->readPRG(address);
    }
    return 0;
}

void MainBus::write(NES_Address address, NES_Byte value) {
    if (address < 0x2000) {
        ram[address & 0x7ff] = value;
    } else if (address < 0x4020) {
        if (address < 0x4000) {  // PPU registers, mirrored
            auto reg = static_cast<IORegisters>(address & 0x2007);
            if (write_callbacks.count(reg))
                return write_callbacks.at(reg)(value);
            else
                LOG(InfoVerbose) << "No write callback registered for I/O register at: " << std::hex << +address << std::endl;
        } else if (address < 0x4017 && address >= 0x4014) {  // only some registers
            auto reg = static_cast<IORegisters>(address);
            if (write_callbacks.count(reg))
                return write_callbacks.at(reg)(value);
            else
                LOG(InfoVerbose) << "No write callback registered for I/O register at: " << std::hex << +address << std::endl;
        } else {
            LOG(InfoVerbose) << "Write access attmept at: " << std::hex << +address << std::endl;
        }
    } else if (address < 0x6000) {
        LOG(InfoVerbose) << "Expansion ROM access attempted. This is currently unsupported" << std::endl;
    } else if (address < 0x8000) {
        if (mapper->hasExtendedRAM())
            extended_ram[address - 0x6000] = value;
    } else {
        mapper->writePRG(address, value);
    }
}

const NES_Byte* MainBus::get_page_pointer(NES_Byte page) {
    NES_Address address = page << 8;
    if (address < 0x2000)
        return &ram[address & 0x7ff];
    else if (address < 0x4020)
        LOG(Error) << "Register address memory pointer access attempt" << std::endl;
    else if (address < 0x6000)
        LOG(Error) << "Expansion ROM access attempted, which is unsupported" << std::endl;
    else if (address < 0x8000)
        if (mapper->hasExtendedRAM())
            return &extended_ram[address - 0x6000];

    return nullptr;
}

void MainBus::set_mapper(Mapper* mapper) {
    this->mapper = mapper;
    if (mapper->hasExtendedRAM())
        extended_ram.resize(0x2000);
}

MainBus::~MainBus() {
    mapper = nullptr;
    write_callbacks.clear();
    read_callbacks.clear();
}

MainBus::MainBus(const MainBus& other) {
    ram = other.ram;
    extended_ram = other.extended_ram;
    mapper = other.mapper;
}

MainBus::MainBus(MainBus&& other) noexcept{
    ram = std::move(other.ram);
    extended_ram = std::move(other.extended_ram);
    mapper = other.mapper;
    other.mapper = nullptr;
    write_callbacks = std::move(other.write_callbacks);  
    read_callbacks = std::move(other.read_callbacks);
}

MainBus& MainBus::operator=(const MainBus& other) {
    if (this == &other)
        return *this;
    ram = other.ram;
    extended_ram = other.extended_ram;
    mapper = other.mapper;
    return *this;
}

MainBus& MainBus::operator=(MainBus&& other) noexcept{
    if (this == &other)
        return *this;
    ram = std::move(other.ram);
    extended_ram = std::move(other.extended_ram);
    mapper = other.mapper;
    other.mapper = nullptr;
    write_callbacks = std::move(other.write_callbacks);  
    read_callbacks = std::move(other.read_callbacks);
    return *this;
}

/// Serializable

void MainBus::serialize(std::vector<uint8_t>& buffer) {
    serialize_vector(ram, buffer);
    serialize_vector(extended_ram, buffer);
}

std::span<uint8_t> MainBus::deserialize(std::span<uint8_t> buffer) {
    // read the RAM
    buffer = deserialize_vector(buffer, ram);
    // read the extended RAM
    buffer = deserialize_vector(buffer, extended_ram);

    return buffer;
}

}  // namespace NES
