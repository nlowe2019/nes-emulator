#include <stdbool.h>
#include <stdint.h>

extern unsigned char ppu_reg[8];
extern bool pause;
extern int steps;
extern int scanline;
extern int dot;
extern uint32_t frame_buffer[240][256];
extern int NMI;
extern uint16_t ppu_address;

enum PPU_STATE
{
    VISIBLE_FRAME,
    POST_RENDER,
    PRE_RENDER,
};

void Update_PPU();
unsigned char read_ppu(uint16_t addr);
void write_ppu(uint16_t addr, unsigned char data);
void OAM_DMA(uint8_t data);

void Draw_Scanline();
void Draw_Nametable();
void Draw_Sprites();
void clear_OAM();
void sprite_eval();
void clear_frame_buffer();
void dump_OAM();
void dump_OAM_secondary();

void increment_vert();
void increment_hori();
void copy_hori();
void copy_vert();