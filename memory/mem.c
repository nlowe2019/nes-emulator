#include <sys/types.h>
#include <stdio.h>
#include "mem.h"
#include "ram.h"
#include "rom.h"
#include "vram.h"
#include "../processing/apu.h"
#include "../processing/ppu.h"
#include "../devices/controller.h"

const int ADDR_RANGE = 0xffff;

unsigned char read(u_int16_t addr) {
    
    unsigned char val;

    if(addr >= 0x2000 && addr < 0x4000) {
        addr = (addr-0x2000) % 8;
        val = read_ppu(addr);
    }
    else if(addr == 0x4016 || addr == 0x4017) {
        val = controller_read(addr);
    }
    else if(addr >= 0x4000 && addr <= 0x4018) {
        val = apu_read(addr);
    }
    else {
        val = *memory_map(addr);
    }
    return val;
}

void write(u_int16_t addr, u_int8_t data) {

    
    if(addr >= 0x2000 && addr < 0x4000) {
        addr = (addr-0x2000) % 8;
        write_ppu(addr, data);
    }
    else if(addr == 0x4014) {
        OAM_DMA(data);
    }
    else if(addr == 0x4016) {
        controller_write(data);
    }
    else if(addr >= 0x4000 && addr <= 0x4018) {
        apu_write(addr, data);
    }
    else if(addr >= 0x8000) {
        mapper_write(addr, data);
    }
    else {
        unsigned char * ptr = memory_map(addr);
        *ptr = data;
    }
}

void Load_Rom(char *file_path) { 
    Parse_Rom(file_path);
}

unsigned char * memory_map(u_int16_t addr) {

    addr %= 0x10000;

    if(addr < 0x2000) {
        return &ram[addr % 0x800];
    }
    else if(addr < 0x4000) {
        return &ppu_reg[(addr-0x2000) % 8];
    }
    else if(addr == 0x4016) {
        return &controller_reg_1;
    }
    else if(addr < 0x4018) {
        return &apu_reg[(addr-0x4000)];
    }
    else if (addr >= 0x6000 && addr < 0x8000) {
        return &mapper->prg_ram[addr-0x6000];
    }
    else if (addr >= 0x8000 && addr <= 0xbfff) {
        return &mapper->rom_data[HEADER_SIZE + (mapper->prg_bank_0 * 0x4000) + (addr - 0x8000)];
    }
    else if (addr >= 0xc000 && addr <= 0xffff) {
        return &mapper->rom_data[HEADER_SIZE + (mapper->prg_bank_1 * 0x4000) + (addr - 0xc000)];
    }
    return &prg_ram[0x1fff];
}

unsigned char read_vram(u_int16_t addr) {

    addr %= 0x4000;

    if(addr < 0x2000 && mapper->chr_length == 0) {
        return mapper->chr_ram[addr];
    }
    
    if(addr < 0x1000) {
        return mapper->rom_data[HEADER_SIZE + ((mapper->prg_ram_length) * 0x4000) + (mapper->chr_bank_0 * 0x1000) + (addr)];
    }
    else if(addr < 0x2000) {
        return mapper->rom_data[HEADER_SIZE + ((mapper->prg_ram_length) * 0x4000) + (mapper->chr_bank_1 * 0x1000) + (addr - 0x1000)];
    }
    else if(addr < 0x3f00) {
        return vram[addr];
    }
    else if(addr < 0x4000) {
        addr = addr % 0x20;
        return vram[addr + 0x3f00];
    }
    return vram[addr];
}

void write_vram(u_int16_t addr, u_int8_t data) {

    addr %= 0x4000;

    if(addr < 0x2000 && mapper->chr_length == 0) {
        mapper->chr_ram[addr] = data;
    }

    else if(addr < 0x3f00) {
        vram[addr] = data;
    }
    else if(addr < 0x4000) {
        addr = addr % 0x20;
        vram[addr + 0x3f00] = data;
    }
}

unsigned char peek_ram(u_int16_t addr) {
    unsigned char val = *memory_map(addr);
    return val;
}

unsigned char peek_vram(u_int16_t addr) {

    addr %= 0x4000;

    if(addr < 0x2000 && mapper->chr_length == 0) {
        return mapper->chr_ram[addr];
    }
    
    if(addr < 0x1000) {
        return mapper->rom_data[HEADER_SIZE + ((mapper->prg_length) * 0x4000) + (mapper->chr_bank_0 * 0x1000) + (addr)];
    }
    else if(addr < 0x2000) {
        return mapper->rom_data[HEADER_SIZE + ((mapper->prg_length) * 0x4000) + (mapper->chr_bank_1 * 0x1000) + (addr - 0x1000)];
    }
    else if(addr < 0x3000) {

        addr -= 0x2000;
        if(mapper->mirroring == 0) {
        int table = 0;
            table = addr / 0x800;
            addr = (table * 0x800) + (addr % 0x400);
        }
        if(mapper->mirroring == 1) {
            addr = addr % 0x800;
        }

        return vram[0x2000 + addr];
    }
    else if(addr < 0x3f00) {
        return vram[addr];
    }
    else if(addr < 0x4000) {
        addr = addr % 0x20;
        return vram[addr + 0x3f00];
    }
    return vram[addr];
}