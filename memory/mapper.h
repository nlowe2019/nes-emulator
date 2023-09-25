#include <sys/types.h>

typedef struct memory_mapper {

    u_int8_t* rom_data;
    u_int8_t prg_ram[0x2000];
    u_int8_t chr_ram[0x2000];

    int type;

    int prg_length; // num of blocks
    int chr_length;
    int prg_ram_length; // block size 8kb

    int prg_ram_bank;
    int prg_bank_0;
    int prg_bank_1;
    int chr_bank_0;
    int chr_bank_1;

    u_int8_t registers[8];

    u_int8_t mirroring;

} memory_mapper;

memory_mapper* create_mapper(u_int8_t* rom_data, int mapper_type, int prg_length, int chr_length, int prg_ram_length, u_int8_t mirroring);

void mapper_write_1(u_int16_t addr, u_int8_t, memory_mapper* mapper);

void mapper_0(memory_mapper* mapper, u_int8_t* rom_buffer);
void mapper_1(memory_mapper* mapper, u_int8_t* rom_buffer);