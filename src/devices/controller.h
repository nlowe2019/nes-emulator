#include <stdint.h>

extern uint8_t controller_reg_1;
extern uint8_t controller_reg_2;

uint8_t controller_read(uint16_t addr);
void controller_write(uint8_t data);

void button_down(int b);
void button_up(int b);
void button_reset();