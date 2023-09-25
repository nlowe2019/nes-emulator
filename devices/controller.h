#include <sys/types.h>
extern u_int8_t controller_reg_1;
extern u_int8_t controller_reg_2;

u_int8_t controller_read(u_int16_t addr);
void controller_write(u_int8_t data);

void button_down(int b);
void button_up(int b);
void button_reset();