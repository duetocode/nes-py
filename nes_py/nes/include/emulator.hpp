//  Program:      nes-py
//  File:         emulator.hpp
//  Description:  This class houses the logic and data for an NES emulator
//
//  Copyright (c) 2019 Christian Kauten. All rights reserved.
//

#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <string>
#include "common.hpp"
#include "cartridge.hpp"
#include "controller.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "main_bus.hpp"
#include "picture_bus.hpp"

namespace NES {

struct SavedState {
    MainBus bus;
    PictureBus picture_bus;
    CPU cpu;
    PPU ppu;
};

/// An NES Emulator and OpenAI Gym interface
class Emulator : public Serializable{
 private:
    /// The number of cycles in 1 frame
    static const int CYCLES_PER_FRAME = 29781;
    /// the virtual cartridge with ROM and mapper data
    Cartridge cartridge;
    /// the mapper
    Mapper* mapper;
    /// the 2 controllers on the emulator
    Controller controllers[2];

    /// the main data bus of the emulator
    MainBus bus;
    /// the picture bus from the PPU of the emulator
    PictureBus picture_bus;
    /// The emulator's CPU
    CPU cpu;
    /// the emulators' PPU
    PPU* ppu;

    SavedState savedState;

    /// @brief setup the callbacks for the internal
    void setup_callbacks();

 public:
    /// The width of the NES screen in pixels
    static const int WIDTH = SCANLINE_VISIBLE_DOTS;
    /// The height of the NES screen in pixels
    static const int HEIGHT = VISIBLE_SCANLINES;

    /// Initialize a new emulator with a path to a ROM file.
    ///
    /// @param rom_path the path to the ROM for the emulator to run
    ///
    explicit Emulator(std::string rom_path, bool headless = false);
    virtual ~Emulator() { delete mapper; delete ppu; }

    /// Return a 32-bit pointer to the screen buffer's first address.
    ///
    /// @return a 32-bit pointer to the screen buffer's first address
    ///
    inline NES_Pixel* get_screen_buffer() { 
        return ppu->get_screen_buffer(); 
    }

    /// Return a 8-bit pointer to the RAM buffer's first address.
    ///
    /// @return a 8-bit pointer to the RAM buffer's first address
    ///
    inline NES_Byte* get_memory_buffer() { return bus.get_memory_buffer(); }

    /// Return a pointer to a controller port
    ///
    /// @param port the port of the controller to return the pointer to
    /// @return a pointer to the byte buffer for the controller state
    ///
    inline NES_Byte* get_controller(int port) {
        return controllers[port].get_joypad_buffer();
    }

    /// Load the ROM into the NES.
    inline void reset() { cpu.reset(bus); ppu->reset(); }

    /// Perform a step on the emulator, i.e., a single frame.
    void step();

    /// Create a backup state on the emulator.
    inline void backup() {
        savedState = *save_state();
    }

    /// Restore the backup state on the emulator.
    inline void restore() {
        load_state(&savedState);
    }

    SavedState* save_state();
    void load_state(SavedState* state);

    void serialize(std::vector<uint8_t>& buffer) override;
    std::span<uint8_t> deserialize(std::span<uint8_t> buffer) override;


};

}  // namespace NES

#endif  // EMULATOR_HPP
