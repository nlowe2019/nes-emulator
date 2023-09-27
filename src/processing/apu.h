#include <stdint.h>
#include <stdio.h>

extern uint8_t apu_reg[0x18];

uint8_t apu_read(uint16_t addr);
void apu_write(uint16_t addr, uint8_t data);