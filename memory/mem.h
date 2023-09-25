#include <sys/types.h>

extern const int ADDR_RANGE;

unsigned char read(u_int16_t addr);
void write(u_int16_t addr, u_int8_t val);
unsigned char * memory_map(u_int16_t addr);

unsigned char read_vram(u_int16_t addr);
void write_vram(u_int16_t addr, u_int8_t data);

unsigned char peek_ram(u_int16_t addr);
unsigned char peek_vram(u_int16_t addr);

void Load_Rom(char *file_path);