#include <stdbool.h>
#include <sys/types.h>

extern unsigned char ppu_reg[8];
extern bool pause;
extern int steps;
extern int scanline;
extern int dot;
extern u_int32_t frame_buffer[240][256];
extern int NMI;
extern u_int16_t ppu_address;

void Update_PPU();
unsigned char read_ppu(u_int16_t addr);
void write_ppu(u_int16_t addr, unsigned char data);
void OAM_DMA(u_int8_t data);

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