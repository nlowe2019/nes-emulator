#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mapper.h"
#include "mem.h"

memory_mapper* create_mapper(uint8_t* rom_data, int mapper_type, int prg_length, int chr_length, int prg_ram_length, uint8_t mirroring) {
    
    memory_mapper* mapper = (memory_mapper*) malloc(sizeof(memory_mapper));

    mapper->rom_data = rom_data;

    mapper->type = mapper_type;
    mapper->prg_length = prg_length;
    mapper->chr_length = chr_length;
    mapper->prg_ram_length = prg_ram_length;

    mapper->prg_ram_bank = 0;
    mapper->prg_bank_0 = 0;
    mapper->prg_bank_1 = 1;

    mapper->mirroring = mirroring;

    return mapper;
}

uint8_t read_rom() {

    return 4;

}

void mapper_0(memory_mapper* mapper, unsigned char* rom_buffer) {
    
    mapper->prg_bank_0 = 0;
    mapper->prg_bank_1 = 0;
    mapper->chr_bank_0 = 0;
    mapper->chr_bank_1 = 1;

    if(mapper->prg_length > 1)
        mapper->prg_bank_1 = 1; 
}

void mapper_1(memory_mapper* mapper, uint8_t* rom_buffer) {

    mapper->prg_bank_0 = 0;
    mapper->prg_bank_1 = mapper->prg_length-1;
    mapper->chr_bank_0 = 0;
    mapper->chr_bank_1 = 1;
    if(mapper->chr_length == 1)
        mapper->chr_bank_1 = 0;

    mapper->registers[0] = 0b10000;
    mapper->registers[1] = 0b01100;
}

void mapper_write_1(uint16_t addr, uint8_t data, memory_mapper* mapper) {

    int SHIFT = 0; int CTRL = 1; int CHR_0 = 2; int CHR_1 = 3; int PRG = 4; int COUNTER = 5;


    if(data & 0b10000000) {
        mapper->registers[SHIFT] = 0b10000;
        mapper->registers[COUNTER] = 0;
        mapper->registers[CTRL] = 0b01100;
    }
    else {
        data = (data & 1) << 4;
        mapper->registers[SHIFT] = (mapper->registers[SHIFT] >> 1) | data;
        mapper->registers[COUNTER]++;

        if(mapper->registers[COUNTER] == 5) {

            if(addr <= 0x9fff) {
                mapper->registers[CTRL] = mapper->registers[SHIFT];
                mapper->mirroring = mapper->registers[CTRL] & 0b11;
            }
            else if(addr <= 0xbfff) {
                mapper->registers[CHR_0] = mapper->registers[SHIFT];
            }
            else if(addr <= 0xdfff) {
                if((mapper->registers[CTRL] >> 4) & 1) {
                    mapper->registers[CHR_1] = mapper->registers[SHIFT];
                    mapper->chr_bank_1 = mapper->registers[CHR_1] & 0b11111;
                }
            }
            else if(addr <= 0xffff) {
                mapper->registers[PRG] = mapper->registers[SHIFT];
                int PRG_BANK_MODE = (mapper->registers[CTRL] >> 2) & 0b11;

                // 32KB bank switch
                if(PRG_BANK_MODE == 0 || PRG_BANK_MODE == 1) {
                    mapper->prg_bank_0 = (mapper->registers[PRG] & 0b1110);
                    mapper->prg_bank_1 = mapper->prg_bank_0 + 1; 
                }
                // first bank fixed, second switched
                else if(PRG_BANK_MODE == 2) {
                    mapper->prg_bank_1 = (mapper->registers[PRG] & 0b1111);
                }
                else if(PRG_BANK_MODE == 3) {
                    mapper->prg_bank_0 = (mapper->registers[PRG] & 0b1111);
                }
            }

            mapper->registers[SHIFT] = 0b10000;
            mapper->registers[COUNTER] = 0;
        }
    }
}

