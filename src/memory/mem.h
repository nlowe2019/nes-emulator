#include <stdint.h>

extern const int ADDR_RANGE;

unsigned char read(uint16_t addr);
void write(uint16_t addr, uint8_t val);
unsigned char * memory_map(uint16_t addr);

unsigned char read_vram(uint16_t addr);
void write_vram(uint16_t addr, uint8_t data);

unsigned char peek_ram(uint16_t addr);
unsigned char peek_vram(uint16_t addr);

void Load_Rom(char *file_path);