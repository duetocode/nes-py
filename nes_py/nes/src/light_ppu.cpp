//  Program:      nes-py
//  File:         light-ppu.cpp
//  Description:  This class is a lightweight implementation.
//
//  Copyright (c) 2024 Zhao Liang. All rights reserved.
//

#include "light_ppu.hpp"
#include "log.hpp"

namespace NES{

void LightPPU::cycle(PictureBus& bus){
    switch (pipeline_state) {
        case PRE_RENDER: {
            if (cycles == 1)
                is_vblank = is_sprite_zero_hit = false;
            else if (cycles == SCANLINE_VISIBLE_DOTS + 2 && is_showing_background && is_showing_sprites) {
                // Set bits related to horizontal position
                data_address &= ~0x41f; //Unset horizontal bits
                data_address |= temp_address & 0x41f; //Copy
            }
            else if (cycles > 280 && cycles <= 304 && is_showing_background && is_showing_sprites) {
                // Set vertical bits
                data_address &= ~0x7be0; //Unset bits related to horizontal
                data_address |= temp_address & 0x7be0; //Copy
            }
            // if (cycles > 257 && cycles < 320)
            //     sprite_data_address = 0;
            // if rendering is on, every other frame is one cycle shorter
            if (cycles >= SCANLINE_END_CYCLE - (!is_even_frame && is_showing_background && is_showing_sprites)) {
                pipeline_state = RENDER;
                cycles = scanline = 0;
            }
            break;
        }
        case RENDER: {
            if (cycles > 0 && cycles <= SCANLINE_VISIBLE_DOTS) {
                NES_Byte bgColor = 0, sprColor = 0;
                bool bgOpaque = false, sprOpaque = true;

                int x = cycles - 1;
                int y = scanline;

                if (is_showing_background) {
                    auto x_fine = (fine_x_scroll + x) % 8;
                    if (!is_hiding_edge_background || x >= 8) {
                        // fetch tile
                        // mask off fine y
                        auto address = 0x2000 | (data_address & 0x0FFF);
                        //auto address = 0x2000 + x / 8 + (y / 8) * (SCANLINE_VISIBLE_DOTS / 8);
                        NES_Byte tile = bus.read(address);

                        //fetch pattern
                        //Each pattern occupies 16 bytes, so multiply by 16
                        //Add fine y
                        address = (tile * 16) + ((data_address >> 12/*y % 8*/) & 0x7);
                        //set whether the pattern is in the high or low page
                        address |= background_page << 12;
                        //Get the corresponding bit determined by (8 - x_fine) from the right
                        //bit 0 of palette entry
                        bgColor = (bus.read(address) >> (7 ^ x_fine)) & 1;
                        //bit 1
                        bgColor |= ((bus.read(address + 8) >> (7 ^ x_fine)) & 1) << 1;

                        //flag used to calculate final pixel with the sprite pixel
                        bgOpaque = bgColor;

                        //fetch attribute and calculate higher two bits of palette
                        address = 0x23C0 | (data_address & 0x0C00) | ((data_address >> 4) & 0x38)
                                    | ((data_address >> 2) & 0x07);
                        auto attribute = bus.read(address);
                        int shift = ((data_address >> 4) & 4) | (data_address & 2);
                        //Extract and set the upper two bits for the color
                        bgColor |= ((attribute >> shift) & 0x3) << 2;
                    }
                    //Increment/wrap coarse X
                    if (x_fine == 7) {
                        // if coarse X == 31
                        if ((data_address & 0x001F) == 31) {
                            // coarse X = 0
                            data_address &= ~0x001F;
                            // switch horizontal nametable
                            data_address ^= 0x0400;
                        }
                        else
                            // increment coarse X
                            data_address += 1;
                    }
                }

                if (is_showing_sprites && (!is_hiding_edge_sprites || x >= 8)) {
                    for (auto i : scanline_sprites) {
                        NES_Byte spr_x =     sprite_memory[i * 4 + 3];

                        if (0 > x - spr_x || x - spr_x >= 8)
                            continue;

                        NES_Byte spr_y     = sprite_memory[i * 4 + 0] + 1,
                            tile      = sprite_memory[i * 4 + 1],
                            attribute = sprite_memory[i * 4 + 2];

                        int length = (is_long_sprites) ? 16 : 8;

                        int x_shift = (x - spr_x) % 8, y_offset = (y - spr_y) % length;

                        if ((attribute & 0x40) == 0) //If NOT flipping horizontally
                            x_shift ^= 7;
                        if ((attribute & 0x80) != 0) //IF flipping vertically
                            y_offset ^= (length - 1);

                        NES_Address address = 0;

                        if (!is_long_sprites) {
                            address = tile * 16 + y_offset;
                            if (sprite_page == HIGH) address += 0x1000;
                        }
                        // 8 x 16 sprites
                        else {
                            //bit-3 is one if it is the bottom tile of the sprite, multiply by two to get the next pattern
                            y_offset = (y_offset & 7) | ((y_offset & 8) << 1);
                            address = (tile >> 1) * 32 + y_offset;
                            address |= (tile & 1) << 12; //Bank 0x1000 if bit-0 is high
                        }

                        sprColor |= (bus.read(address) >> (x_shift)) & 1; //bit 0 of palette entry
                        sprColor |= ((bus.read(address + 8) >> (x_shift)) & 1) << 1; //bit 1

                        if (!(sprOpaque = sprColor)) {
                            sprColor = 0;
                            continue;
                        }

                        sprColor |= 0x10; //Select sprite palette
                        sprColor |= (attribute & 0x3) << 2; //bits 2-3

                        //Sprite-0 hit detection
                        if (!is_sprite_zero_hit && is_showing_background && i == 0 && sprOpaque && bgOpaque)
                            is_sprite_zero_hit = true;

                        break; //Exit the loop now since we've found the highest priority sprite
                    }
                }
            }
            else if (cycles == SCANLINE_VISIBLE_DOTS + 1 && is_showing_background) {
                //Shamelessly copied from nesdev wiki
                if ((data_address & 0x7000) != 0x7000) {  // if fine Y < 7
                    // increment fine Y
                    data_address += 0x1000;
                } else {
                    // fine Y = 0
                    data_address &= ~0x7000;
                    // let y = coarse Y
                    int y = (data_address & 0x03E0) >> 5;
                    if (y == 29) {
                        // coarse Y = 0
                        y = 0;
                        // switch vertical nametable
                        data_address ^= 0x0800;
                    } else if (y == 31) {
                        // coarse Y = 0, nametable not switched
                        y = 0;
                    } else {
                        // increment coarse Y
                        y += 1;
                    }
                    // put coarse Y back into data_address
                    data_address = (data_address & ~0x03E0) | (y << 5);
                }
            }
            else if (cycles == SCANLINE_VISIBLE_DOTS + 2 && is_showing_background && is_showing_sprites) {
                // Copy bits related to horizontal position
                data_address &= ~0x41f;
                data_address |= temp_address & 0x41f;
            }

//                 if (cycles > 257 && cycles < 320)
//                     sprite_data_address = 0;

            if (cycles >= SCANLINE_END_CYCLE) {
                //Find and index sprites that are on the next Scanline
                //This isn't where/when this indexing, actually copying in 2C02 is done
                //but (I think) it shouldn't hurt any games if this is done here

                scanline_sprites.resize(0);

                int range = 8;
                if (is_long_sprites)
                    range = 16;

                NES_Byte j = 0;
                for (NES_Byte i = sprite_data_address / 4; i < 64; ++i) {
                    auto diff = (scanline - sprite_memory[i * 4]);
                    if (0 <= diff && diff < range) {
                        scanline_sprites.push_back(i);
                        if (++j >= 8)
                            break;
                    }
                }

                ++scanline;
                cycles = 0;
            }

            if (scanline >= VISIBLE_SCANLINES)
                pipeline_state = POST_RENDER;

            break;
        }
        case POST_RENDER: {
            if (cycles >= SCANLINE_END_CYCLE) {
                ++scanline;
                cycles = 0;
                pipeline_state = VERTICAL_BLANK;
            }
            break;
        }
        case VERTICAL_BLANK: {
            if (cycles == 1 && scanline == VISIBLE_SCANLINES + 1) {
                is_vblank = true;
                if (is_interrupting) vblank_callback();
            }

            if (cycles >= SCANLINE_END_CYCLE) {
                ++scanline;
                cycles = 0;
            }

            if (scanline >= FRAME_END_SCANLINE) {
                pipeline_state = PRE_RENDER;
                scanline = 0;
                is_even_frame = !is_even_frame;
                // is_vblank = false;
            }

            break;
        }
        default:
            LOG(Error) << "Well, this shouldn't have happened." << std::endl;
    }
    ++cycles;
}
}