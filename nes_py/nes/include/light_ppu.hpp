//  Program:      nes-py
//  File:         light-ppu.hpp
//  Description:  Lightweight PPU simulation does not rendering 
//
//  Copyright (c) 2024 Zhao Liang. All rights reserved.
//

#ifndef LIGHT_PPU_HPP
#define LIGHT_PPU_HPP

#include "ppu.hpp"

namespace NES {


class LightPPU : public PPU {
public:
    void cycle(PictureBus& bus) override;
};

}

#endif