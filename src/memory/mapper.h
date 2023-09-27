#include <stdint.h>

typedef struct memory_mapper {

    uint8_t* rom_data;
    uint8_t prg_ram[0x2000];
    uint8_t chr_ram[0x2000];

    int type;

    int prg_length; // num of blocks
    int chr_length;
    int prg_ram_length; // block size 8kb

    int prg_ram_bank;
    int prg_bank_0;
    int prg_bank_1;
    int chr_bank_0;
    int chr_bank_1;

    uint8_t registers[8];

    uint8_t mirroring;

} memory_mapper;

memory_mapper* create_mapper(uint8_t* rom_data, int mapper_type, int prg_length, int chr_length, int prg_ram_length, uint8_t mirroring);

void mapper_write_1(uint16_t addr, uint8_t, memory_mapper* mapper);

void mapper_0(memory_mapper* mapper, uint8_t* rom_buffer);
void mapper_1(memory_mapper* mapper, uint8_t* rom_buffer);