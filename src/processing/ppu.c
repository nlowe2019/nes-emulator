#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../memory/mem.h"
#include "../memory/ram.h"
#include "ppu.h"
#include "palette.h"

// --PPU registers (in CPU addressable memory)--

// cpu address offset - 0x2000 (0x0001 -> 0x2001)

/*
NMI enable: V
PPU master/slave: P
Spirte height: H
Background tile select: B
Sprite tile select: S
Increment mode: I
Nametable Select: NN
*/
const int PPUCTRL = 0x0000; //VPHB SINN
const uint8_t NMI_BIT = 0b10000000;
const uint8_t MASTER_SLAVE_BIT = 0b01000000;
const uint8_t SPRITE_HEIGHT_BIT = 0b00100000;
const uint8_t BACKGROUND_PATTERN_TABLE_BIT = 0b00010000;
const uint8_t SPRITE_PATTERN_TABLE_BIT = 0b00001000;
const uint8_t ADDR_INCREMENT_BIT = 0b00000100;
const uint8_t NAME_TABLE_2_BIT = 0b00000010;
const uint8_t NAME_TABLE_1_BIT = 0b00000001;

/*
Color emphasis: BGR
Sprite enable: s
Background enable: b
Sprite left column enable: M
Background left column enable: m
Greyscale: G
*/
const int PPUMASK = 0x0001; //BGRs bMmG
const uint8_t GREYSCALE_BIT = 0b00000001;
const uint8_t SHOW_BG_LEFT_BIT = 0b00000010;
const uint8_t SHOW_SPRITES_LEFT_BIT = 0b00000100;
const uint8_t SHOW_BG_BIT = 0b00001000;
const uint8_t SHOW_SPRITES_BIT = 0b00010000;
const uint8_t EMPHASIS_RED_BIT = 0b00100000;
const uint8_t EMPHASIS_GREEN_BIT = 0b01000000;
const uint8_t EMPHASIS_BLUE_BIT = 0b10000000;

/*
vBlank: V
Sprite 0 hit: S
Sprite overflow: O
*/
const int PPUSTATUS = 0x0002; //VSO- ----
const uint8_t V_BLANK_BIT = 0b10000000;
const uint8_t S0_HIT_BIT = 0b01000000;
const uint8_t SPRITE_OVERFLOW_BIT = 0b00100000;

const int OAMADDR = 0x0003; //aaaa aaaa | OAM read/write address
const int OAMDATA = 0x0004; //dddd dddd | OAM data read/write
const int PPUSCROLL = 0x0005; //xxxx xxxx | fine scroll position (two writes x, y)
const int PPUADDR = 0x0006; //aaaa aaaa | PPU read/write address (two writes, high nibble, low nibble)
const int PPUDATA = 0x0007; //dddd dddd | PPU data read/write

uint8_t ppu_reg[0x8];
uint8_t OAM_memory[0x100];
uint8_t OAM_memory_secondary[0x40];

uint32_t frame_buffer[240][256]; 
uint8_t data_buffer = 0;

uint16_t ppu_address_temp;  // 't' register
uint16_t ppu_address;  // 'v' register
uint8_t fine_x = 0;   // 'x' register

int rendering_enabled = 0;

int frame = 0;
int scanline = -1;
int dot = 0;
int latch = 0;  // 'w register'
int NMI = 0;

uint8_t sprite_priority[240][256];

int render_s0 = 0;

void Update_PPU() {

    if(scanline == -1) {

        //idle
    }
    else if(scanline < 240) {

        // visible scanlines
        Draw_Scanline();
    }
    else if(scanline == 240) {

        // idle
    }
    else if(scanline < 261) { 

        // post-render scanlines
        if(scanline == 241 && dot == 1) {
            // VBlank period begins
            ppu_reg[PPUSTATUS] |= V_BLANK_BIT; 
        }
        if(scanline == 241 && dot == 1) {
            // active NMI (non maskable interrupt)
            if(ppu_reg[PPUCTRL] & NMI_BIT) {
                NMI = 1;
            }
        }
    }
    else {

        // pre-render line, switch off status flags
        if(dot == 1) {
            render_s0 = 0;
            ppu_reg[PPUSTATUS] &= ~S0_HIT_BIT;
            ppu_reg[PPUSTATUS] &= ~SPRITE_OVERFLOW_BIT;
            ppu_reg[PPUSTATUS] &= ~V_BLANK_BIT;
        }
        if(((dot > 0 && dot <= 256)/* || dot >= 328 */) && dot%8 == 0) {
            increment_hori();
        }
        if(dot == 256) {
            increment_vert();
        }
        if(dot == 257) {
            copy_hori();
        }
        if(dot >= 280 && dot <= 304) {
            copy_vert();
        }
        if(dot == 339) { 
            // ensures NMI is disabled
            NMI = 0;

            // on odd frames, first dot is skipped
            if(frame % 2 == 1) {

                dot = 0;
                scanline = 0;
            } 
        }
    }

    dot++;
    if(dot > 340) {

        scanline++;
        //end of frame
        if(scanline > 261) {
            frame++;
            scanline = 0;
        }
        dot = 0;
    }

    //if(scanline == 0 && dot == 100)
    //    dump_OAM();
    /*if(scanline == 30) {
        ppu_reg[PPUSTATUS] |= S0_HIT_BIT;
    } else {
        ppu_reg[PPUSTATUS] &= 0xbf;
    }*/
}

uint8_t read_ppu(uint16_t addr) {

    uint8_t return_data;

    switch(addr) {

        case 0x00:
            return ppu_reg[PPUADDR];
            break;

        case 0x01:
            return ppu_reg[PPUMASK];
            break;

        case 0x02:
            return_data = ppu_reg[PPUSTATUS];

            ppu_reg[PPUSTATUS] &= ~V_BLANK_BIT; // VERT BLANK BIT CLEARED AFTER $2002 READ
            latch = 0;

            return return_data;
            break;

        case 0x03:
            return ppu_reg[OAMADDR];
            break;

        case 0x04:
            if(dot < 65 && scanline < 240)
                return 0xff;
            return OAM_memory[ppu_reg[OAMADDR]];
            break;

        case 0x05:
            return ppu_reg[PPUSCROLL];
            break;

        case 0x06: 
            return ppu_reg[PPUADDR];
            break;

        case 0x07:
                
            return_data = data_buffer;
            data_buffer = read_vram(ppu_address);

            int increment = (ppu_reg[PPUCTRL]) & ADDR_INCREMENT_BIT ? 32 : 1;
            ppu_address += increment;
            ppu_reg[PPUADDR] += increment;

            if(ppu_address >= 0x3f00) {
                data_buffer = read_vram(ppu_address - 0x1000);
                return read_vram(ppu_address);
            }

            return return_data;
            break;
    }
    return 0;
}

void write_ppu(uint16_t addr, uint8_t data) {

    switch(addr) {

        case 0x00:
            ppu_reg[PPUCTRL] = data;
            ppu_address_temp &= 0b1110011111111111;
            ppu_address_temp |= ((data & 0b11) << 11);
            break;

        case 0x01:
            ppu_reg[PPUMASK] = data;
            if((data & 0x10) || (data & 0x08))
                rendering_enabled = 1;
            else
                rendering_enabled = 0;
            break;

        case 0x02:
            ppu_reg[PPUSTATUS] = data;
            break;

        case 0x03:
            ppu_reg[OAMADDR] = data;
            break;

        case 0x04:
            if(scanline < 262) {
                OAM_memory[ppu_reg[OAMADDR]] = data;
                //printf("Writing %02x to OAM addr: %02x\n", data, ppu_reg[OAMADDR]);
                ppu_reg[OAMADDR]++;
                ppu_reg[OAMADDR] &= 0xff;
            }
            break;

        case 0x05:
            if(latch == 0) {
                fine_x = data & 0b111;
                ppu_address_temp &= ~0b11111;
                ppu_address_temp |= (data >> 3);
                latch = 1;
            }
            else {
                ppu_address_temp &= 0b000110000011111;
                ppu_address_temp |= ((data & 0b111) << 12);
                ppu_address_temp |= (((data & 0b11111000) >> 3) << 5);
                latch = 0;
            }
            break;

        case 0x06:
            if(latch == 0) {
                ppu_address_temp &= ~0b111111100000000;
                ppu_address_temp |= ((data & 0b111111) << 8);
                latch = 1;
            } else {
                ppu_address_temp &= 0xff00;
                ppu_address_temp |= (data & 0xff);
                ppu_address = ppu_address_temp;
                latch = 0;
            }
            //printf("\nppu_addr: %04x\n\n", ppu_address);
            break;

        case 0x07: 
            if(1/*(ppu_reg[PPUSTATUS] & V_BLANK_BIT) || !((ppu_reg[PPUMASK] & SHOW_BG_BIT) || (ppu_reg[PPUMASK] & SHOW_SPRITES_BIT))*/) { //unsure on logic, commented code created problems
                write_vram(ppu_address, data);
                //printf("\nWriting %02x to addr: %04x\n\n", data, ppu_address);
                    
                int increment = (ppu_reg[PPUCTRL]) & ADDR_INCREMENT_BIT ? 32 : 1;
                ppu_address += increment;
                ppu_reg[PPUADDR] += increment;
            }
            break;
    }
}

void Draw_Scanline() {

    if(dot == 1) {
        clear_OAM();
    }
    else if(dot == 65) {
        sprite_eval();
        //dump_OAM_secondary();
    }

    if(((dot > 0 && dot <= 256))&& dot%8 == 0) {
        increment_hori();
    }

    if(dot%8 == 1 && dot < 255) {
        if(ppu_reg[PPUMASK] & SHOW_BG_BIT) {
            Draw_Nametable();
        }
    }
    if(dot == 255) {
        if(ppu_reg[PPUMASK] & SHOW_SPRITES_BIT && scanline < 239) {
            Draw_Sprites();
        }
    }
    if(dot == 256) {
        increment_vert();
    }
    if(dot == 257) {
        copy_hori();
    }
}

void Draw_Sprites() {
    
    uint8_t sprite_y;
    uint8_t pattern_index;
    uint8_t attributes;
    uint8_t sprite_x;
    uint8_t sprite_lower;
    uint8_t sprite_upper;
    SDL_Color color;

    for(int sprite_num = 0; sprite_num < 32; sprite_num+=4) {

        sprite_y = OAM_memory_secondary[sprite_num];
        pattern_index = OAM_memory_secondary[sprite_num+1];
        attributes = OAM_memory_secondary[sprite_num+2];
        sprite_x = OAM_memory_secondary[sprite_num+3];

        int pattern_table_addr;
        if(ppu_reg[PPUCTRL] & SPRITE_HEIGHT_BIT) {  // 8x16 sprites

            pattern_table_addr = pattern_index & 0x1 ? 0x1000 : 0x0000;
            int sprite_row = scanline+1 - sprite_y;

            if(attributes & 0b10000000) { 
                sprite_row = sprite_row % 8;
                sprite_row = 7 - sprite_row;
                if(scanline+1 - sprite_y < 8) {
                    sprite_row += 16;
                }
            }
            sprite_lower = read_vram((0x10 * pattern_index) + pattern_table_addr + sprite_row);
            sprite_upper = read_vram((0x10 * pattern_index) + pattern_table_addr  + 8 + sprite_row);
        }
        else {  // 8x8 sprites

            pattern_table_addr = (ppu_reg[PPUCTRL] & SPRITE_PATTERN_TABLE_BIT) ? 0x1000 : 0x0000;
            int sprite_row = scanline+1 - sprite_y;
            if(attributes & 0b10000000) {
                sprite_row = 7 - sprite_row;
            }
            sprite_lower = read_vram((0x10 * pattern_index) + pattern_table_addr + sprite_row);
            sprite_upper = read_vram((0x10 * pattern_index) + pattern_table_addr  + 8 + sprite_row);
        }

        int palette_id = attributes & 0b11;
        uint16_t palette_addr = 0x3f00 + 0x10 + (4 * palette_id);

        //if(sprite_x != 255)
        //    printf("sprite index = oam2[%d]: %d, addr:%04x, row: %d\n", sprite_num, pattern_index, (0x1 * pattern_index) + pattern_table_addr + sprite_row, sprite_row);
        if(sprite_y > 0 && sprite_y < 0xf0) {
            for(int pixel = 0; pixel < 8; pixel++) {

                uint8_t val; 
                if(attributes & 0b01000000) {
                    val = (sprite_lower >> (pixel)) & 0b1;
                    val += ((sprite_upper >> (pixel)) & 0b1) << 1;
                }
                else {
                    val = (sprite_lower >> (7-pixel)) & 0b1;
                    val += ((sprite_upper >> (7-pixel)) & 0b1) << 1;
                }

                color = palette[read_vram(palette_addr + val)];
                
                if(sprite_x + pixel < 0xff && sprite_y < 0xff) {
                    if(sprite_priority[scanline+1][sprite_x+pixel] == 0) {
                        frame_buffer[scanline+1][sprite_x+pixel] = 0xff000000 + (color.r << 16);
                        frame_buffer[scanline+1][sprite_x+pixel] += (color.g << 8);
                        frame_buffer[scanline+1][sprite_x+pixel] += (color.b);
                    }
                }

                if(!(attributes & 0b00100000) && val > 0 || sprite_priority[scanline+1][sprite_x+pixel] == 1)
                    sprite_priority[scanline+1][sprite_x+pixel] = 1;
                else if ((attributes & 0b00100000) && val > 0) {
                    sprite_priority[scanline+1][sprite_x+pixel] = 2;
                }
                else
                    sprite_priority[scanline+1][sprite_x+pixel] = 0;
            }
        }
    }
}

void Draw_Nametable() {

    SDL_Color color;

    /*
    Read pattern table id from nametable to find sprite
    */
    int NAME_TABLE_ID = ppu_reg[0] & NAME_TABLE_1_BIT ? 1 : 0;
    int section = (dot/8) * 8;
    int pattern_id = peek_vram(0x2000 | (ppu_address & 0xfff));

    /*
    Calculate address in VRAM of sprite and then the byte
    needed for the current scanline. One bit of each pixel
    is stored in seperate bytes.
    */
    int pattern_table_addr = (ppu_reg[PPUCTRL] & BACKGROUND_PATTERN_TABLE_BIT) ? 0x1000 : 0x0000;
    int pattern_addr = pattern_id * 0x10;
    pattern_addr += pattern_table_addr;
    pattern_addr += scanline % 8;

    uint8_t pattern_byte = peek_vram(pattern_addr);
    uint8_t pattern_byte_2 = peek_vram(pattern_addr + 8);

    for(int i = 0; i < 8; i++) { 

        /*
        Each pixel is assigned a 2-bit value (0-3). The value is matched to one of
        four colours from a colour palette.
        */

        int palette_tile_x = dot / 32;
        int palette_tile_y = scanline / 32;
        int quadrant = (dot % 32) < 16 ? 0b0 : 0b1;
        quadrant += (scanline % 32) < 16 ? 0b00 : 0b10;
        
        uint16_t attr_addr = 0x23c0 + palette_tile_x + (8 * palette_tile_y);
        //uint16_t attr_addr = 0x23c0 | (ppu_address & 0x0c00) | ((ppu_address >> 4) & 0x38) | ((ppu_address >> 2) & 0x07);

        uint8_t palette_id = read_vram(attr_addr);
        palette_id = (palette_id >> (2 * quadrant)) & 0b11;
        uint16_t palette_addr = 0x3f00 + (4*palette_id);
    
        uint8_t value = (pattern_byte >> (7-(i%8))) & 0b1;
        value += ((pattern_byte_2 >> (7-(i%8))) & 0b1) << 1;

        if(value == 0)
            color = palette[read_vram(0x3f00)];
        else
            color = palette[read_vram(palette_addr + value)];

        if(sprite_priority[scanline][section+i] == 0 || (sprite_priority[scanline][section+i] == 2 && value > 0)) {
            frame_buffer[scanline][section+i] = 0xff000000 + (color.r << 16);
            frame_buffer[scanline][section+i] += (color.g << 8);
            frame_buffer[scanline][section+i] += (color.b);
        }
        else {
            if(value > 0 && !(ppu_reg[PPUSTATUS] & S0_HIT_BIT) && render_s0) {
                //printf("SPRITE 0 HIT at frame [%d] line [%d]\n", frame, scanline);
                ppu_reg[PPUSTATUS] |= S0_HIT_BIT; 
            }
        }
    }
}

void OAM_DMA(uint8_t data) {

    uint16_t ram_addr = 0x100 * data;
    uint8_t oam_addr = ppu_reg[OAMADDR];

    for(int i = 0; oam_addr + i < 256; i++) { 

        //printf("Writing %02x to OAM[0x%02x]\n", read(ram_addr + i), oam_addr + i);
        OAM_memory[oam_addr + i] = read(ram_addr + i);
    }
}

void clear_OAM(void) {

    for(int i = 0; i < 8*4; i++) {
        OAM_memory_secondary[i] = 0xff;
    }
}

void sprite_eval() {

    int n = 0; 
    for(int i = 0; i < 64; i++) {

        int height_offset = 7;
        if(ppu_reg[PPUCTRL] & SPRITE_HEIGHT_BIT)
            height_offset = 15;
        
        if((OAM_memory[i*4]) > 0 && (OAM_memory[i*4]-1) <= scanline && (OAM_memory[i*4]-1)+height_offset >= scanline && (OAM_memory[i*4]-1) < 240) {

            if(n == 8) {
                ppu_reg[PPUSTATUS] |= SPRITE_OVERFLOW_BIT;
            }
            else {
                
                if(i == 0 && !render_s0) {
                    render_s0 = 1;
                }

                OAM_memory_secondary[(n*4)] = OAM_memory[(i*4)];
                OAM_memory_secondary[(n*4) + 1] = OAM_memory[(i*4) + 1];
                OAM_memory_secondary[(n*4) + 2] = OAM_memory[(i*4) + 2];
                OAM_memory_secondary[(n*4) + 3] = OAM_memory[(i*4) + 3];
                n++;
            }
        }
    }
}

void clear_frame_buffer() {
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j< 240; j++) {
            frame_buffer[j][i] = 0;
            sprite_priority[j][i] = 0;
        }
    }
}

void dump_OAM() {
    
    printf("OAM MEMORY:\n");
    for (int i = 0; i < 256; i+=4) {
        printf("SPRITE[%d]   %02x %02x %02x %02x\n", i/4, OAM_memory[i+0], OAM_memory[i+1], OAM_memory[i+2], OAM_memory[i+3]);
    }
    printf("\n");
}

void dump_OAM_secondary() {
    
    printf("SECONDARY OAM MEMORY:\n");
    for (int i = 0; i < 64; i+=4) {
        printf("SPRITE[%d]   %02x %02x %02x %02x\n", i/4, OAM_memory_secondary[i+0], OAM_memory_secondary[i+1], OAM_memory_secondary[i+2], OAM_memory_secondary[i+3]);
    }
    printf("\n");
}

void increment_hori() {
    
    if(rendering_enabled) {
        if((ppu_address & 0x001f) == 31) {
            ppu_address &= ~0x001f;
            ppu_address ^= 0x0400;
        } else {
            ppu_address++;
        }
    }
}

void increment_vert() {

    if(rendering_enabled) {
        if((ppu_address & 0x7000) != 0x7000) {
            ppu_address += 0x1000;
        }
        else {
            ppu_address &= ~0x7000;
            int y = (ppu_address & 0x03e0) >> 5;
            if(y == 29) {
                y = 0;
                ppu_address ^= 0x0800;
            }
            else if(y == 31) {
                y = 0;
            }
            else {
                y++;
            }

            ppu_address = (ppu_address & ~0x03e0) | (y << 5);
        }
    }
}

void copy_hori() {

    if(rendering_enabled) {
        uint16_t temp_copy = ppu_address_temp;
        temp_copy &= 0b10000011111;
        ppu_address &= ~0b000010000011111;
        ppu_address |= temp_copy;
    }
}

void copy_vert() {

    if(rendering_enabled) {
        uint16_t temp_copy = ppu_address_temp;
        temp_copy &= 0b111101111100000;
        ppu_address &= ~0b111101111100000;
        ppu_address |= temp_copy;
    }
}