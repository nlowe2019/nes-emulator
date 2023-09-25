#include <sys/types.h>
#include <stdio.h>

extern u_int8_t apu_reg[0x18];

u_int8_t apu_read(u_int16_t addr);
void apu_write(u_int16_t addr, u_int8_t data);