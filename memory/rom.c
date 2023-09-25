#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "rom.h"
#include "vram.h"

// starting point in RAM for ROM data
int prg_rom_cpu_addr;
int prg_rom_cpu_length;
int prg_rom_mirror_offset = 0x8000;

int prg_ram_cpu_addr;
int prg_ram_cpu_length;
int chr_rom_ppu_addr;
int chr_rom_ppu_addr;

const int HEADER_SIZE = 0x10;
const int TRAINER_BLOCK_SIZE = 0x200;
const int PRG_ROM_BLOCK_SIZE = 0x4000;
const int CHR_ROM_BLOCK_SIZE = 0x2000;

memory_mapper* mapper;

int file_size;

const int FLAGS6 = 6;
unsigned char mapper_lower; // 
bool trainer; // 512 byte (0x200) trainer at $7000-$71ff
bool battery; // battery powered prg ram $6000-$7fff (used for save memory)
bool ignore_mirror; // ignore mirror bit and use 4 screen vram
bool mirror; // 0: horizontal, 1: vertical

const int FLAGS7 = 7;
bool vs;
bool playchoice;
bool nes2;
unsigned char mapper_upper;

const int FLAGS8 = 8;
unsigned char submapper;

const int FLAGS10 = 10;
bool tv_system; // 0: NTSC 1: PAL
bool prg_ram_exists;
bool bus_conflicts;

unsigned char prg_ram[0x2000];

void Parse_Rom(char *file_path) { 

    remove("log.txt");
    FILE* file_ptr = fopen("log.txt", "w");
    fclose(file_ptr);

    unsigned char* rom_data;

    FILE *rom_file = fopen(file_path, "r"); 
    if(rom_file) {
        file_size = fsize(rom_file);
        rom_data = (unsigned char*)malloc(sizeof(char)*file_size);
        fread(rom_data, sizeof(char), file_size, rom_file);
    } else {
        printf("\nROM file not laoded\n");
    }
    fclose(rom_file); 

    int prg_length = rom_data[4];
    int chr_length = rom_data[5];

    u_int8_t mirror = rom_data[FLAGS6] & (1<<0) ? true : false;
    battery = rom_data[FLAGS6] & (1<<1) ? true : false;
    trainer = rom_data[FLAGS6] & (1<<2) ? true : false;
    ignore_mirror = rom_data[FLAGS6] & (1<<3) ? true : false;
    mapper_lower = rom_data[FLAGS6] >> 4;

    vs = rom_data[FLAGS7] & (1<<0) ? true : false;
    playchoice = rom_data[FLAGS7] & (1<<0) ? true : false;
    nes2 = ((rom_data[FLAGS7] >> 2) & 3) == 2 ? true : false;
    mapper_upper = rom_data[FLAGS7] >> 4;

    submapper = rom_data[FLAGS8] >> 0;
    printf("submapper: %d\n", submapper);
    printf("NES2: %d\n", nes2);

    tv_system = (rom_data[FLAGS10] & 3) == 2 ? true : false;
    prg_ram_exists = (rom_data[FLAGS10] >> 4 ) & 1 ? false : true;

    mapper = create_mapper(rom_data, mapper_lower + (mapper_upper << 8), prg_length, chr_length, 1, mirror);

    int prg_rom_offset = HEADER_SIZE + (trainer ? TRAINER_BLOCK_SIZE : 0);
    int chr_rom_offset = prg_rom_offset + (PRG_ROM_BLOCK_SIZE * prg_length);

    switch(mapper->type) {
        case 0:
            mapper_0(mapper, rom_data);
            break;
        case 1:
            mapper_1(mapper, rom_data);
            break;
        default:
            printf("-------------------- MAPPER NOT IMPLEMENTED ------------------------");
    }

    ROM_Description(file_path);
}

// helper

int fsize(FILE *fp) {
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}

// ShutDown

void Shut_Down_ROM() {
    free(mapper->rom_data);
    free(mapper);
}

void ROM_Description(char* filepath) {
    printf("ROM: %s\nPRG LENGTH: %d\nCHR LENGTH: %d\nMAPPER: %d\nmirror: %d, trainer: %d, battery: %d, ignore_mirror: %d, PRG_RAM: %d\n\n"
        , filepath, mapper->prg_length, mapper->chr_length, mapper_lower, mirror, trainer, battery, ignore_mirror, prg_ram_exists);
}

void mapper_write(u_int16_t addr, u_int8_t data) {

    switch (mapper->type) {
        
        case 1:
            mapper_write_1(addr, data, mapper);
            break;

    }
}