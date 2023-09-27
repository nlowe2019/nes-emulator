#include <stdio.h>
#include <stdint.h>

#include "mapper.h"

extern const int HEADER_SIZE;
extern const int TRAINER_BLOCK_SIZE;
extern const int PRG_ROM_BLOCK_SIZE;
extern const int CHR_ROM_BLOCK_SIZE;

extern unsigned char* prg_rom;
extern unsigned char* chr_rom;
extern unsigned char prg_ram[0x2000];

extern int prg_rom_cpu_addr;
extern int prg_rom_cpu_length;
extern int prg_rom_mirror_offset;

extern int prg_ram_cpu_addr;
extern int prg_ram_cpu_length;

extern memory_mapper* mapper;

void Parse_Rom(char *file_path);

void ROM_Description(char* filepath);
int fsize(FILE *fp);

void mapper_write(uint16_t addr, uint8_t data);

void Shut_Down_ROM();