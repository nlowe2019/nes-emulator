#include <stdbool.h>
#include <stdint.h>

extern unsigned char ppu_reg[8];
extern int scanline;
extern int dot;
extern uint32_t frame_buffer[240][256];
extern int generate_nmi;
extern uint16_t ppu_address;

enum PPU_STATE
{
    VISIBLE_FRAME,
    POST_RENDER,
    PRE_RENDER,
};

void ppu_update();
unsigned char ppu_read(uint16_t reg_num);
void ppu_write(uint16_t reg_num, uint8_t data);
void OAM_DMA(uint8_t data);
void draw_name_table_byte();
void fetch_sprite_data();
void draw_sprites();
void clear_OAM();
void sprite_evaluation();
void clear_frame_buffer(); 
void increment_vertical();
void increment_horizontal();
void copy_horizontal();
void copy_vertical();