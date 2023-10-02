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

uint32_t frame_buffer[240][256]; 
uint8_t ppu_reg[0x8];

uint16_t temp_ppu_address;
uint16_t ppu_address;
/*
yyy NN YYYYY XXXXX
||| || ||||| +++++-- coarse X scroll
||| || +++++-------- coarse Y scroll
||| ++-------------- nametable select
+++----------------- fine Y scroll
*/
uint8_t fine_x_scoll;
uint8_t buffer_data;
int latch = 0;

uint8_t OAM_memory[0x100];
uint8_t OAM_memory_secondary[0x40];
uint8_t sprite_patterns_lower[8];
uint8_t sprite_patterns_upper[8];
uint8_t sprite_attributes[8];
int sprite_counters[8];

enum PPU_STATE ppu_state = PRE_RENDER;
int scanline = -1;
int dot = 0;

int generate_nmi = 0;
int sprite_0_loaded = 0;

void ppu_update()
{
    switch (ppu_state)
    {
        case PRE_RENDER: // line -1/261
            if(dot == 1)
                ppu_reg[PPUSTATUS] &= ~(V_BLANK_BIT | S0_HIT_BIT | SPRITE_OVERFLOW_BIT);
            if(dot >= 280 && dot <= 304)
                copy_vertical();
            if(dot == 256)
                increment_vertical();
            if((dot == 328 || dot == 336 || (dot >= 8 && dot <= 256)) && (dot%8) == 0 )
                increment_horizontal();
            if(dot == 257)
                copy_horizontal(); 
            if(dot >= 257 && dot <= 320)
                ppu_reg[OAMADDR] = 0;
            break;

        case VISIBLE_FRAME: // line 0 - 240
            if(dot == 1)
                fetch_sprite_data();
            if(dot == 64)
                clear_OAM();
            if(dot == 256)
                increment_vertical();
            if((dot == 328 || dot == 336 || (dot >= 8 && dot <= 256)) && (dot%8) == 0 )
                increment_horizontal();
            if(dot == 257)
            {
                copy_horizontal(); 
                sprite_evaluation();
            }
            if(dot >= 257 && dot <= 320)
                ppu_reg[OAMADDR] = 0;
            if(dot < 256 && (dot % 8) == 1)
                draw_name_table_byte();
            if(dot < 256)
                draw_sprites();
            break; 

        case POST_RENDER: // line 241 - 260
            if(scanline == 241 && dot == 1)
            {
                ppu_reg[PPUSTATUS] |= V_BLANK_BIT; 
                if(ppu_reg[PPUCTRL] & NMI_BIT)
                    generate_nmi = 1;
            }
            break; 
    }

    dot = (dot > 339) ? 0 : dot + 1;
    if(dot == 0)
        scanline = (scanline > 260) ? 0 : scanline + 1;
    if(scanline == 0)
        ppu_state = PRE_RENDER;
    else if(scanline < 240)
        ppu_state = VISIBLE_FRAME;
    else
        ppu_state = POST_RENDER;

}

uint8_t ppu_read(uint16_t reg_num)
{
    uint8_t buffer_return;
    switch (reg_num)
    {
        case 2: // STATUS
            ppu_reg[PPUSTATUS] &= ~V_BLANK_BIT;
            latch = 0;
            break;

        case 4: // OAMDATA
            return ppu_reg[OAMDATA];
            break;

        case 7: // DATA
            buffer_return = buffer_data;
            buffer_data = read_vram(ppu_address);
            int increment = (ppu_reg[PPUCTRL]) & ADDR_INCREMENT_BIT ? 32 : 1;
            ppu_address += increment;
            ppu_reg[PPUADDR] += increment;

            if(ppu_address >= 0x3f00) {
                buffer_data = read_vram(ppu_address - 0x1000);
                return read_vram(ppu_address);
            }
            return buffer_return; 
            break;
        
        default:
            break;
    }
}

void ppu_write(uint16_t reg_num, uint8_t data)
{
    switch (reg_num)
    {
        case 0: // CTRL
            temp_ppu_address &= ~(0b11 << 10);
            temp_ppu_address |= ((data & 0b11) << 10);

            latch = 0;
            ppu_reg[PPUCTRL] = data;
            break;

        case 1: // MASK

            ppu_reg[PPUMASK] = data;
            break;

        case 3: // OMA ADDR
            ppu_reg[OAMADDR] = data;
            break;

        case 4: // OMA DATA
            ppu_reg[OAMDATA] = data;
            ppu_reg[OAMADDR]++;
            break;

        case 5: // SCROLL
            if(!latch)
            {
                temp_ppu_address &= ~(0b11111);
                temp_ppu_address |= (data & 0b11111);
                fine_x_scoll = (data & 0b111);
            }
            else
            {
                temp_ppu_address &= ~(0b11111 << 5);
                temp_ppu_address |= ((data & 0b11111) << 5);
                temp_ppu_address &= ~(0b111 << 12);
                temp_ppu_address |= ((data & 0b111) << 12);
            }
            latch = !latch;
            ppu_reg[PPUSCROLL] = data;
            break;

        case 6: // ADDR
            if(!latch)
            {
                temp_ppu_address &= ~(0b111111);
                temp_ppu_address |= (data & 0b111111) << 8;
                temp_ppu_address &= ~(1 << 14);
            }
            else
            {
                temp_ppu_address &= ~(0b11111111);
                temp_ppu_address |= (data & 0b11111111);
                ppu_address = temp_ppu_address;
            }
            latch = !latch;
            ppu_reg[PPUADDR] = data;
            break;

        case 7: // DATA

            ppu_reg[PPUDATA] = data;
            ppu_address += (ppu_reg[PPUCTRL] & ADDR_INCREMENT_BIT) ? 32 : 1;
            break;

        case 14: // OMA_DMA
            OAM_DMA(data);
            break;

        default:
            break;
    }
}

void increment_vertical() 
{
    if ((ppu_address & 0x7000) != 0x7000)             // if fine Y < 7
    {
        ppu_address += 0x1000;                        // increment fine Y
    }
    else
    {
        ppu_address &= ~0x7000;                       // fine Y = 0
        int y = (ppu_address & 0x03E0) >> 5;          // let y = coarse Y
        if (y == 29)
        {
            y = 0;                                    // coarse Y = 0
            ppu_address ^= 0x0800;                    // switch vertical nametable
        }
        else if (y == 31)
        {
            y = 0;                                    // coarse Y = 0, nametable not switched
        }
        else
        {
            y += 1;                                   // increment coarse Y
        }
        ppu_address = (ppu_address & ~0x03E0) | (y << 5);    // put coarse Y back into v
    }
}

void increment_horizontal()
{
    if ((ppu_address & 0x001F) == 31)    // if coarse X == 31
    {
        ppu_address &= ~0x001F;          // coarse X = 0
        ppu_address ^= 0x0400;           // switch horizontal nametable
    }
    else
    {
        ppu_address += 1;                // increment coarse X
    }
}

void copy_vertical()
{
    uint16_t vertical_mask = 0b111101111100000;
    ppu_address &= ~vertical_mask;
    ppu_address |= (temp_ppu_address & vertical_mask);
}

void copy_horizontal()
{
    uint16_t name_table_select = temp_ppu_address & (1 << 10);
    uint16_t coarse_x = temp_ppu_address & 0b11111;
    ppu_address &= ~0b10000011111;
    ppu_address |= coarse_x;
    ppu_address |= name_table_select;

}

void OAM_DMA(uint8_t data)
{
    uint16_t ram_addr = 0x100 * data;
    uint8_t oam_addr = ppu_reg[OAMADDR];

    for(int i = 0; oam_addr + i < 256; i++) {
        //printf("Writing %02x to OAM[0x%02x]\n", read(ram_addr + i), oam_addr + i);
        OAM_memory[oam_addr + i] = read(ram_addr + i);
    }
}

void clear_frame_buffer() 
{
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j< 240; j++) {
            frame_buffer[j][i] = 0;
        }
    }
}

void clear_OAM(void)
{
    for(int i = 0; i < 64; i++) {
        OAM_memory_secondary[i] = 0xff;
    }
}

void draw_name_table_byte()
{
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

        if(value > 0) {
            frame_buffer[scanline][section+i] = 0xff000000 + (color.r << 16);
            frame_buffer[scanline][section+i] += (color.g << 8);
            frame_buffer[scanline][section+i] += (color.b);
        }
    }
}

/*
cycle through each sprite in primary oam memory.
if y co-ordinate puts sprite on next scanline, add to oam secondary
*/
void sprite_evaluation()
{
    int n = 0; // oam secondary index
    int oam_size = 256;

    for(int i = 0; i < sizeof(OAM_memory); i+=4) 
    {
        /*
        sprite height, determined by CTRL register
        */
        int height_offset = 7;
        if(ppu_reg[PPUCTRL] & SPRITE_HEIGHT_BIT)
            height_offset = 15;

        int sprite_y = OAM_memory[i];

        if(sprite_y > 0 && sprite_y-1 <= scanline && sprite_y-1+height_offset >= scanline && sprite_y-1 < 240) {

            if(n == 8)
            {
                ppu_reg[PPUSTATUS] |= SPRITE_OVERFLOW_BIT;
            }
            else
            {
                if(i == 0)
                    sprite_0_loaded = 1;
                OAM_memory_secondary[(n*4)] = OAM_memory[i];
                OAM_memory_secondary[(n*4) + 1] = OAM_memory[i + 1];
                OAM_memory_secondary[(n*4) + 2] = OAM_memory[i + 2];
                OAM_memory_secondary[(n*4) + 3] = OAM_memory[i + 3];
                n++;
            }
        }

    }
}

void fetch_sprite_data()
{
    for(int i = 0; i < 8; i++)
    {
        sprite_patterns_lower[i] = 0xff;
        sprite_patterns_upper[i] = 0xff;
        sprite_attributes[i] = 0xff;
        sprite_counters[i] =-8;
    }

    // max 8 sprites per scanline
    for(int i = 0; i < 8; i++)
    {
        if(OAM_memory_secondary[i*4] != 0xff)
        {
            int sprite_y = OAM_memory_secondary[i];
            uint16_t sprite_pattern_index = OAM_memory_secondary[(i*4)+1];
            sprite_attributes[i] = OAM_memory_secondary[i+2];
            sprite_counters[i] = OAM_memory_secondary[i+3];

            uint16_t chr_bank_addr;
            int sprite_row = scanline+1 - sprite_y;

            if(ppu_reg[PPUCTRL] & SPRITE_HEIGHT_BIT)   // 8x16 sprites
            {
                chr_bank_addr = sprite_pattern_index & 1 ? 0x1000 : 0x0000;

                if(sprite_attributes[i] & 0b10000000) { 
                    sprite_row = sprite_row % 8;
                    sprite_row = 7 - sprite_row;
                    if(scanline+1 - sprite_y < 8) {
                        sprite_row += 16;
                    }
                }
            }
            else   // 8x8 sprites
            {
                chr_bank_addr = (ppu_reg[PPUCTRL] & SPRITE_PATTERN_TABLE_BIT) ? 0x1000 : 0x0000;
                if(sprite_attributes[i] & 0b10000000) {
                    sprite_row = 7 - sprite_row;
                }
            }
            sprite_patterns_lower[i] = read_vram((0x10 * sprite_pattern_index) + chr_bank_addr + sprite_row);
            sprite_patterns_upper[i] = read_vram((0x10 * sprite_pattern_index) + chr_bank_addr + sprite_row + 8);
        }
    }
}

void draw_sprites()
{
    SDL_Color pixel_color;

    for(int i = 0; i < 8; i++)
    {
        sprite_counters[i]--;

        if(sprite_counters[i] <= 0 && sprite_counters[i] >= -7)
        {
            
            int bit_position = abs(sprite_counters[i]);
            if(sprite_attributes[i] & 0b01000000)
                bit_position = 7 - bit_position;

            int sprite_bit_low = 1 & (sprite_patterns_lower[i] >> bit_position);
            int sprite_bit_upper = 1 & (sprite_patterns_upper[i] >> bit_position);
            int pattern_bits = sprite_bit_low + (sprite_bit_upper << 1);

            int palette_id = sprite_attributes[i] & 0b11;
            uint16_t palette_addr = 0x3f00 + 0x10 + (4 * palette_id);
            pixel_color = palette[read_vram(palette_addr + pattern_bits)];
            
            if(i == 0 && sprite_0_loaded)
            {
                int bg_transparent = 1;
                SDL_Color transparent_pixel_color = palette[read_vram(0x3f00)];
                uint32_t transparent_color_32 = 0xff000000; + (pixel_color.r << 16);
                transparent_color_32 += (pixel_color.g << 8);
                transparent_color_32 += (pixel_color.b);
                if(frame_buffer[scanline][dot] != transparent_color_32)
                    bg_transparent = 0;
                if(!bg_transparent && pattern_bits > 0)
                {
                    ppu_reg[PPUSTATUS] |= S0_HIT_BIT;
                    sprite_0_loaded = 0;
                }
            }

            if(!(sprite_attributes[i] & 0b00100000))   // render behind background attribute
            {    
                frame_buffer[scanline][dot] = 0xff000000 + (pixel_color.r << 16);
                frame_buffer[scanline][dot] += (pixel_color.g << 8);
                frame_buffer[scanline][dot] += (pixel_color.b);
            }
            
        }
    }
}