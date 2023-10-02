#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../nes.h"
#include "cpu.h"
#include "../memory/mem.h"
#include "ppu.h"

// registers
uint16_t pc = 0;
uint8_t sp = 0xfd;
uint8_t status = 0b100;

uint8_t accumulator = 0;
uint8_t index_x = 0;
uint8_t index_y = 0;

void Init_CPU() {

    pc = read(0xfffc);
    pc += (read(0xfffd) << 8);

    printf("Program Counter initialised to 0x%04x\n", pc);
}

int Update_CPU() {

    if(generate_nmi) {

        status |= 0b00010000;
        StackPush(status);
        StackPush(pc >> 8);
        StackPush(pc);

        pc = read(0xfffa);
        pc += (read(0xfffb) << 8);
        generate_nmi = 0;
    }

    uint8_t opcode = FetchInstruction();
    struct opcode op = OpcodeLookup(opcode);
    if((pc & 0xfff0) == 0xfff0)
        pause = true;
    return ExecuteInstruction(op);
}

uint8_t FetchInstruction() {

    uint8_t opcode = read(pc);
    pc ++;
    return opcode;
}

opcode OpcodeLookup(uint8_t opcode) {

    for(int i = 0; i < 256; i++) {
        if (opcode_table[i].OPCODE == opcode) {
            return opcode_table[i];
        }
    }
    return opcode_table[0];
}

int ExecuteInstruction(opcode opcode) {
 
    int cycles = opcode.cycles;
    int page_cross = 0;

    enum INSTRUCTION instruction = opcode.INSTRUCTION;
    enum ADDRESS_MODE mode = opcode.MODE;
    uint16_t addr = GetAddress(mode, &page_cross);

//    FILE* file_ptr = fopen("log.txt", "a");
/*
    if (mode == IMMEDIATE || mode == ZERO_PAGE || mode == ZERO_PAGE_X || mode == INDIRECT_X || mode == INDIRECT_Y || mode == ZERO_PAGE_Y || mode == RELATIVE ) {
        printf("[%04x] %s: %s (%02x), %02x\n", pc-2, get_address_mode_string((uint8_t)mode), get_opcode_name(opcode.OPCODE), instruction, read(pc-1));
  //      fprintf(file_ptr, "\n%04x  %02x %02x     %s  %04x", pc-2, read(pc-2), read(pc-1), get_opcode_name(opcode.OPCODE), addr);
    }
    else if (mode == IMPLIED || mode == ACCUMULATOR ) {
        printf("[%04x] %s: %s (%02x)\n", pc-1, get_address_mode_string((uint8_t)mode), get_opcode_name(opcode.OPCODE), instruction);
    //    fprintf(file_ptr, "\n%04x  %02x        %s  %04x", pc-1, read(pc-1), get_opcode_name(opcode.OPCODE), addr);
    }
    else {
        printf("[%04x] %s: %s (%02x), %02x %02x\n", pc-3, get_address_mode_string((uint8_t)mode), get_opcode_name(opcode.OPCODE), instruction, read(pc-1), read(pc-2));
      //  fprintf(file_ptr, "\n%04x  %02x %02x %02x  %s  %04x", pc-3, read(pc-3), read(pc-2), read(pc-1), get_opcode_name(opcode.OPCODE), addr);
    }
*/
    //fprintf(file_ptr, "        A:%02x X:%02x Y:%02x S:%02x SP:%02x CYC:%d", accumulator, index_x, index_y, status, sp, c%341);

    //fclose(file_ptr);

    switch (instruction) {

        case ADC:
            cycles += adc6502(mode, addr);
            cycles += page_cross;
            break;
        case AND:
            cycles += and6502(mode, addr);
            cycles += page_cross;
            break;
        case ASL:
            cycles += asl6502(mode, addr);
            break;
        case BCC:
            cycles += bcc6502(mode, addr);
            break;
        case BCS:
            cycles += bcs6502(mode, addr);
            break;
        case BEQ:
            cycles += beq6502(mode, addr);
            break;
        case BIT:
            cycles += bit6502(mode, addr);
            break;
        case BMI:
            cycles += bmi6502(mode, addr);
            break;
        case BNE:
            cycles += bne6502(mode, addr);
            break;
        case BPL:
            cycles += bpl6502(mode, addr);
            break;
        case BRK:
            cycles += brk6502(mode, addr);
            break;
        case BVC:
            cycles += bvc6502(mode, addr);
            break;
        case BVS:
            cycles += bvs6502(mode, addr);
            break;
        case CLC:
            cycles += clc6502(mode, addr);
            break;
        case CLD:
            cycles += cld6502(mode, addr);
            break;
        case CLI:
            cycles += cli6502(mode, addr);
            break;
        case CLV:
            cycles += clv6502(mode, addr);
            break;
        case CMP:
            cycles += cmp6502(mode, addr);
            cycles += page_cross;
            break;
        case CPX:
            cycles += cpx6502(mode, addr);
            break;
        case CPY:
            cycles += cpy6502(mode, addr);
            break;
        case DEC:
            cycles += dec6502(mode, addr);
            break;
        case DEX:
            cycles += dex6502(mode, addr);
            break;
        case DEY:
            cycles += dey6502(mode, addr);
            break;
        case EOR:
            cycles += eor6502(mode, addr);
            cycles += page_cross;
            break;
        case INC:
            cycles += inc6502(mode, addr);
            break;
        case INX:
            cycles += inx6502(mode, addr);
            break;
        case INY:
            cycles += iny6502(mode, addr);
            break;
        case JMP:
            cycles += jmp6502(mode, addr);
            break;
        case JSR:
            cycles += jsr6502(mode, addr);
            break;
        case LDA:
            cycles += lda6502(mode, addr);
            cycles += page_cross;
            break;
        case LDX:
            cycles += ldx6502(mode, addr);
            cycles += page_cross;
            break;
        case LDY:
            cycles += ldy6502(mode, addr);
            cycles += page_cross;
            break;
        case LSR:
            cycles += lsr6502(mode, addr);
            break;
        case NOP:
            cycles += nop6502(mode, addr);
            break;
        case ORA:
            cycles += ora6502(mode, addr);
            cycles += page_cross;
            break;
        case PHA:
            cycles += pha6502(mode, addr);
            break;
        case PHP:
            cycles += php6502(mode, addr);
            break;
        case PLA:
            cycles += pla6502(mode, addr);
            break;
        case PLP:
            cycles += plp6502(mode, addr);
            break;
        case ROL:
            cycles += rol6502(mode, addr);
            break;
        case ROR:
            cycles += ror6502(mode, addr);
            break;
        case RTI:
            cycles += rti6502(mode, addr);
            break;
        case RTS:
            cycles += rts6502(mode, addr);
            break;
        case SBC:
            cycles += sbc6502(mode, addr);
            cycles += page_cross;
            break;
        case SEC:
            cycles += sec6502(mode, addr);
            break;
        case SED:
            cycles += sed6502(mode, addr);
            break;
        case SEI:
            cycles += sei6502(mode, addr);
            break;
        case STA:
            cycles += sta6502(mode, addr);
            break;
        case STX:
            cycles += stx6502(mode, addr);
            break;
        case STY:
            cycles += sty6502(mode, addr);
            break;
        case TAX:
            cycles += tax6502(mode, addr);
            break;
        case TAY:
            cycles += tay6502(mode, addr);
            break;
        case TSX:
            cycles += tsx6502(mode, addr);
            break;
        case TXA:
            cycles += txa6502(mode, addr);
            break;
        case TXS:
            cycles += txs6502(mode, addr);
            break;
        case TYA:
            cycles += tya6502(mode, addr);
            break;
    }

    return cycles;
}

// address modes

uint16_t GetAddress(enum ADDRESS_MODE mode, int* page_cross) {
    
    uint16_t addr = 0x0000;
    uint16_t ptr = 0x0000;

    switch (mode) {

        case IMPLIED:
            break;

        case ACCUMULATOR:
            break;

        case IMMEDIATE:
            addr = read(pc);
            pc ++;
            break;

        case ZERO_PAGE:
            addr = read(pc);
            addr &= 0x00ff;
            pc ++;
            break;

        case ZERO_PAGE_X:
            addr = read(pc) + index_x;
            addr &= 0xff;
            pc ++;
            break;

        case ZERO_PAGE_Y:
            addr = read(pc) + index_y;
            addr &= 0xff;
            pc ++;
            break;

        case ABSOLUTE:
            addr = read(pc);
            pc ++;
            addr += (read(pc) << 8);
            pc ++;
            break;

        case ABSOLUTE_X:
            addr = read(pc);
            pc ++;
            addr += (read(pc) << 8);
            pc ++;

            if((addr & 0xff) + index_x > 255) {
                (*page_cross)++;
            }
            addr += index_x;
            break;

        case ABSOLUTE_Y:
            addr = read(pc);
            pc ++;
            addr += (read(pc) << 8);
            pc ++;

            if((addr & 0xff) + index_y > 255) {
                (*page_cross)++;
            }
            addr += index_y;
            break;

        case INDIRECT:
            ptr = read(pc) + (read(pc+1) << 8);
            addr = read(ptr);
            pc ++;
            if((ptr & 0xff) == 0xff)
                addr += read(ptr-0xff) << 8;
            else
                addr += read(ptr+1) << 8;
            pc ++;
            break;

        case INDIRECT_X:
            ptr = (read(pc) + index_x) & 0xff;
            addr = read(ptr);
            addr += read((uint8_t)(ptr + 1)) << 8;
            pc ++;
            break;

        case INDIRECT_Y:
            ptr = read(pc);
            addr = read(ptr);
            addr += read((uint8_t)(ptr+1)) << 8;

            if((addr & 0xff) + index_y > 255) {
                (*page_cross)++;
            }
            addr += index_y; 
            pc ++;
            break;

        case RELATIVE:
            addr = read(pc);
            pc ++;
            break;
    }
    return addr;
}

// instructions

int adc6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t accumulator_initial = accumulator;
    uint8_t value = (mode == IMMEDIATE) ? addr : read(addr);

    uint16_t result = accumulator + value + (status & FLAG_CARRY ? 1 : 0);
    accumulator = result & 0xff;
 
    SetFlag(FLAG_CARRY, result > 255);

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, accumulator & FLAG_NEGATIVE);

    SetFlag(FLAG_OVERFLOW, (accumulator_initial ^ accumulator) & (value ^ accumulator) & 0x80);

    return 0;
}

int and6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == IMMEDIATE ? addr : read(addr));

    accumulator &= value;

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, accumulator & FLAG_NEGATIVE); 

    return 0;
}

int asl6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == ACCUMULATOR) ? accumulator : read(addr);

    SetFlag(FLAG_CARRY, (value << 1) > 255);

    value <<= 1;

    SetFlag(FLAG_ZERO, value == 0);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & value) == FLAG_NEGATIVE);

    if(mode == ACCUMULATOR) 
        accumulator = value;
    else
        write(addr, value);

    return 0;
}

/*
branch if carry flag 0. requires more cycles if successful
and also if pc moves to different page.
*/
int bcc6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (!(status & FLAG_CARRY)) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int bcs6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (status & FLAG_CARRY) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int beq6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (status & FLAG_ZERO) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if branches to different page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int bit6502(enum ADDRESS_MODE mode, uint16_t addr) {
    
    uint8_t value = read(addr);

    SetFlag(FLAG_NEGATIVE, value & FLAG_NEGATIVE);
    SetFlag(FLAG_OVERFLOW, value & FLAG_OVERFLOW);

    SetFlag(FLAG_ZERO, !(value & accumulator));
    
    return 0;
}

int bmi6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (status & FLAG_NEGATIVE) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int bne6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (!(status & FLAG_ZERO)) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;
        return extra_cycles;
    }

    return 0;
}

int bpl6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (!(status & FLAG_NEGATIVE)) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int brk6502(enum ADDRESS_MODE mode, uint16_t addr) {
    
    StackPush(pc >> 8);
    StackPush(pc);
    status |= 0b00110000;
    StackPush(status);

    pc = read(0xfffe);
    pc += read(0xffff) << 8;

    SetFlag(FLAG_BREAK, true);

    return 0;
}

int bvc6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (!(status & FLAG_OVERFLOW)) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int bvs6502(enum ADDRESS_MODE mode, uint16_t addr) {

    char pc_displacement = (char) addr;

    if (status & FLAG_OVERFLOW) {

        int extra_cycles = ((0xff00 & pc) != (0xff00 & (pc + pc_displacement))) ? 2 : 1; // if pc moves to new page
        pc += pc_displacement;

        return extra_cycles;
    }

    return 0;
}

int cmp6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == IMMEDIATE) ? addr : read(addr);

    SetFlag(FLAG_CARRY, accumulator >= value); 
    SetFlag(FLAG_ZERO, accumulator == value);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & (accumulator - value)) == FLAG_NEGATIVE);

    return 0;
}

int cpx6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == IMMEDIATE ? addr : read(addr));

    SetFlag(FLAG_CARRY, index_x >= value);
    SetFlag(FLAG_ZERO, index_x == value);
    SetFlag(FLAG_NEGATIVE, (index_x - value) & FLAG_NEGATIVE);

    return 0;    
}

int cpy6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == IMMEDIATE) ? addr : read(addr);

    SetFlag(FLAG_CARRY, index_y >= value);
    SetFlag(FLAG_ZERO, index_y == value);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & (index_y - value));

    return 0;
}

int dec6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = read(addr);
    value--;
    write(addr, value);

    SetFlag(FLAG_ZERO,!value);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & value);

    return 0;
}

int dex6502(enum ADDRESS_MODE mode, uint16_t addr) {
    
    index_x--;

    SetFlag(FLAG_ZERO, index_x == 0);
    SetFlag(FLAG_NEGATIVE, index_x & FLAG_NEGATIVE);

    return 0;
}

int dey6502(enum ADDRESS_MODE mode, uint16_t addr) {
    
    index_y--;

    SetFlag(FLAG_ZERO, index_y == 0);
    SetFlag(FLAG_NEGATIVE, index_y & FLAG_NEGATIVE);

    return 0;
}

int eor6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == IMMEDIATE) ? addr : read(addr);

    accumulator ^= value;

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, accumulator & FLAG_NEGATIVE);

    return 0;
}

int inc6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = read(addr);
    value++;
    write(addr, value);

    SetFlag(FLAG_ZERO, value == 0);
    SetFlag(FLAG_NEGATIVE, value & FLAG_NEGATIVE);

    return 0;
}

int inx6502(enum ADDRESS_MODE mode, uint16_t addr) {

    index_x++;

    SetFlag(FLAG_ZERO, index_x == 0);
    SetFlag(FLAG_NEGATIVE, index_x & FLAG_NEGATIVE);

    return 0;
}

int iny6502(enum ADDRESS_MODE mode, uint16_t addr) {

    index_y++;

    SetFlag(FLAG_ZERO, !index_y);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & index_y);

    return 0;
}

int jmp6502(enum ADDRESS_MODE mode, uint16_t addr) {

    pc = addr;

    return 0;
}

int jsr6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = read(addr);

    pc--;
    StackPush(pc >> 8);
    StackPush(pc);
    pc = addr;

    return 0;
}

int lda6502(enum ADDRESS_MODE mode, uint16_t addr) {

    accumulator = ((mode == IMMEDIATE) ? addr : read(addr));

    SetFlag(FLAG_ZERO, !accumulator);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & accumulator); 
    
    return 0;
}

int ldx6502(enum ADDRESS_MODE mode, uint16_t addr) {
 
    index_x = (mode == IMMEDIATE) ? addr : read(addr);

    SetFlag(FLAG_ZERO, !index_x);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & index_x) == FLAG_NEGATIVE); 
    
    return 0;
}

int ldy6502(enum ADDRESS_MODE mode, uint16_t addr) {

    index_y = (mode == IMMEDIATE) ? addr : read(addr);

    SetFlag(FLAG_ZERO, !index_y);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & index_y) == FLAG_NEGATIVE); 
    
    return 0;
}

int lsr6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = read(addr);

    if(mode == ACCUMULATOR)
        value = accumulator;

    status &= ~FLAG_CARRY;
    status |= (FLAG_CARRY & value);

    value >>= 1;

    SetFlag(FLAG_ZERO, !value);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & value) == FLAG_NEGATIVE); 

    if(mode == ACCUMULATOR)
        accumulator = value;
    else
        write(addr, value);

    return 0;
}

int nop6502(enum ADDRESS_MODE mode, uint16_t addr) {

    return 0;
}

int ora6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == IMMEDIATE) ? addr : read(addr);

    accumulator |= value;

    SetFlag(FLAG_ZERO, !accumulator);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & accumulator) == FLAG_NEGATIVE); 

    return 0;
}

/*
pushes copy of the accumulator to stack
*/
int pha6502(enum ADDRESS_MODE mode, uint16_t addr) {

    StackPush(accumulator);

    return 0;
}

int php6502(enum ADDRESS_MODE mode, uint16_t addr) {

    status |= 0b00110000;
    StackPush(status);

    return 0;
}

int pla6502(enum ADDRESS_MODE mode, uint16_t addr) {

    accumulator = StackPop();

    SetFlag(FLAG_ZERO, !accumulator);
    SetFlag(FLAG_NEGATIVE, (FLAG_NEGATIVE & accumulator) == FLAG_NEGATIVE); 

    return 0;
}

int plp6502(enum ADDRESS_MODE mode, uint16_t addr) {

    status = StackPop();

    return 0;
}

int rol6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == ACCUMULATOR ? accumulator : read(addr));

    uint8_t carry = (status & FLAG_CARRY ? 1 : 0);
    SetFlag(FLAG_CARRY, value & 0b10000000);

    value = value << 1;
    value |= carry;

    if(mode == ACCUMULATOR) {
        accumulator = value;
    } else {
        write(addr, value);
    }

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, value & FLAG_NEGATIVE);

    return 0;
}

int ror6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t value = (mode == ACCUMULATOR ? accumulator : read(addr));

    uint8_t carry = (status & FLAG_CARRY ? 0b10000000 : 0);
    SetFlag(FLAG_CARRY, value & 0b00000001);

    value = value >> 1;
    value |= carry;

    if(mode == ACCUMULATOR) {
        accumulator = value;
    } else {
        write(addr, value);
    }

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, value & FLAG_NEGATIVE);

    return 0;
}

int rti6502(enum ADDRESS_MODE mode, uint16_t addr) {

    pc = StackPop();
    pc += StackPop() << 8;
    status = StackPop();

    return 0;
}

int rts6502(enum ADDRESS_MODE mode, uint16_t addr) {

    pc = StackPop();
    pc += StackPop() << 8;

    pc++;

    return 0;
}

int sbc6502(enum ADDRESS_MODE mode, uint16_t addr) {

    uint8_t accumulator_initial = accumulator;
    uint8_t value = (mode == IMMEDIATE ? addr : read(addr));
    value ^= 0xff;

    uint16_t result = accumulator + value + (status & FLAG_CARRY ? 1 : 0);
    accumulator = result & 0xff;
 
    SetFlag(FLAG_CARRY, result > 255);

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, accumulator & FLAG_NEGATIVE);

    SetFlag(FLAG_OVERFLOW, (accumulator_initial ^ accumulator) & (value ^ accumulator) & 0x80);

    return 0;
}

int sta6502(enum ADDRESS_MODE mode, uint16_t addr) {

    write(addr, accumulator);
    
    if(addr == 0x4014)
        return 513;
    return 0;
}

int stx6502(enum ADDRESS_MODE mode, uint16_t addr) {

    write(addr, index_x);

    if(addr == 0x4014)
        return 513;
    return 0;
}

int sty6502(enum ADDRESS_MODE mode, uint16_t addr) {

    write(addr, index_y);

    if(addr == 0x4014)
        return 513;
    return 0;
}

int tax6502(enum ADDRESS_MODE mode, uint16_t addr) {

    index_x = accumulator;

    SetFlag(FLAG_ZERO, index_x == 0);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & index_x); 

    return 0;
}

int tay6502(enum ADDRESS_MODE mode, uint16_t addr) {
    
    index_y = accumulator;

    SetFlag(FLAG_ZERO, index_y == 0);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & index_y); 

    return 0;
}

int tsx6502(enum ADDRESS_MODE mode, uint16_t addr) {

    index_x = sp;

    SetFlag(FLAG_ZERO, index_x == 0);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & index_x); 

    return 0;
}

int txa6502(enum ADDRESS_MODE mode, uint16_t addr) {

    accumulator = index_x;

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, FLAG_NEGATIVE & accumulator); 

    return 0;
}

int txs6502(enum ADDRESS_MODE mode, uint16_t addr) {

    sp = index_x;

    return 0;
}

int tya6502(enum ADDRESS_MODE mode, uint16_t addr) {

    accumulator = index_y;

    SetFlag(FLAG_ZERO, accumulator == 0);
    SetFlag(FLAG_NEGATIVE, accumulator & FLAG_NEGATIVE); 

    return 0;
}

int sec6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_CARRY, 1);
    return 0;
}

int sed6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_DECIMAL, 1);
    return 0;
}

int sei6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_INTERRUPT, 1);
    return 0;
}

int clc6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_CARRY, 0);
    return 0;
}

int cld6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_DECIMAL, 0);
    return 0;
}

int cli6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_INTERRUPT, 0);
    return 0;
}

int clv6502(enum ADDRESS_MODE mode, uint16_t addr) {
    SetFlag(FLAG_OVERFLOW, 0);
    return 0;
}

// set flags

void SetFlag(int flag, bool condition) {

    if(condition)
        status |= flag;
    else
        status &= ~flag;
}

// stack functions

void StackPush(uint8_t value) {

    write(0x100 + sp, value);
    sp--;
    sp = (uint8_t)sp;
}

uint8_t StackPop(void) {

    sp++;
    sp = (uint8_t)sp;
    return read(0x100 + sp);
}


// opcode lists for easy lookups

opcode opcode_table[256] = {
    {0x69, IMMEDIATE, ADC, 2}, {0x65, ZERO_PAGE, ADC, 3}, {0x75, ZERO_PAGE_X, ADC, 4}, {0x6D, ABSOLUTE, ADC, 4},
    {0x7D, ABSOLUTE_X, ADC, 4}, {0x79, ABSOLUTE_Y, ADC, 4}, {0x61, INDIRECT_X, ADC, 6}, {0x71, INDIRECT_Y, ADC, 5},
    {0x29, IMMEDIATE, AND, 2}, {0x25, ZERO_PAGE, AND, 3}, {0x35, ZERO_PAGE_X, AND, 4}, {0x2D, ABSOLUTE, AND, 4},
    {0x3D, ABSOLUTE_X, AND, 4}, {0x39, ABSOLUTE_Y, AND, 4}, {0x21, INDIRECT_X, AND, 6}, {0x31, INDIRECT_Y, AND, 5},
    {0x0A, ACCUMULATOR, ASL, 2}, {0x06, ZERO_PAGE, ASL, 5}, {0x16, ZERO_PAGE_X, ASL, 6}, {0x0E, ABSOLUTE, ASL, 6},
    {0x1E, ABSOLUTE_X, ASL, 7}, {0x90, RELATIVE, BCC, 2}, {0xB0, RELATIVE, BCS, 2}, {0xF0, RELATIVE, BEQ, 2},
    {0x24, ZERO_PAGE, BIT, 3}, {0x2C, ABSOLUTE, BIT, 4}, {0x30, RELATIVE, BMI, 2}, {0xD0, RELATIVE, BNE, 2},
    {0x10, RELATIVE, BPL, 2}, {0x00, IMPLIED, BRK, 7}, {0x50, RELATIVE, BVC, 2}, {0x70, RELATIVE, BVS, 2},
    {0x18, IMPLIED, CLC, 2}, {0xD8, IMPLIED, CLD, 2}, {0x58, IMPLIED, CLI, 2}, {0xB8, IMPLIED, CLV, 2},
    {0xC9, IMMEDIATE, CMP, 2}, {0xC5, ZERO_PAGE, CMP, 3}, {0xD5, ZERO_PAGE_X, CMP, 4}, {0xCD, ABSOLUTE, CMP, 4},
    {0xDD, ABSOLUTE_X, CMP, 4}, {0xD9, ABSOLUTE_Y, CMP, 4}, {0xC1, INDIRECT_X, CMP, 6}, {0xD1, INDIRECT_Y, CMP, 5},
    {0xE0, IMMEDIATE, CPX, 2}, {0xE4, ZERO_PAGE, CPX, 3}, {0xEC, ABSOLUTE, CPX, 4}, {0xC0, IMMEDIATE, CPY, 2},
    {0xC4, ZERO_PAGE, CPY, 3}, {0xCC, ABSOLUTE, CPY, 4}, {0xC6, ZERO_PAGE, DEC, 5}, {0xD6, ZERO_PAGE_X, DEC, 6},
    {0xCE, ABSOLUTE, DEC, 6}, {0xDE, ABSOLUTE_X, DEC, 7}, {0xCA, IMPLIED, DEX, 2}, {0x88, IMPLIED, DEY, 2},
    {0x49, IMMEDIATE, EOR, 2}, {0x45, ZERO_PAGE, EOR, 3}, {0x55, ZERO_PAGE_X, EOR, 4}, {0x4D, ABSOLUTE, EOR, 4},
    {0x5D, ABSOLUTE_X, EOR, 4}, {0x59, ABSOLUTE_Y, EOR, 4}, {0x41, INDIRECT_X, EOR, 6}, {0x51, INDIRECT_Y, EOR, 5},
    {0xE6, ZERO_PAGE, INC, 5}, {0xF6, ZERO_PAGE_X, INC, 6}, {0xEE, ABSOLUTE, INC, 6}, {0xFE, ABSOLUTE_X, INC, 7},
    {0xE8, IMPLIED, INX, 2}, {0xC8, IMPLIED, INY, 2}, {0x4C, ABSOLUTE, JMP, 3}, {0x6C, INDIRECT, JMP, 5},
    {0x20, ABSOLUTE, JSR, 6}, {0xA9, IMMEDIATE, LDA, 2}, {0xA5, ZERO_PAGE, LDA, 3}, {0xB5, ZERO_PAGE_X, LDA, 4},
    {0xAD, ABSOLUTE, LDA, 4}, {0xBD, ABSOLUTE_X, LDA, 4}, {0xB9, ABSOLUTE_Y, LDA, 4}, {0xA1, INDIRECT_X, LDA, 6},
    {0xB1, INDIRECT_Y, LDA, 5}, {0xA2, IMMEDIATE, LDX, 2}, {0xA6, ZERO_PAGE, LDX, 3}, {0xB6, ZERO_PAGE_Y, LDX, 4},
    {0xAE, ABSOLUTE, LDX, 4}, {0xBE, ABSOLUTE_Y, LDX, 4}, {0xA0, IMMEDIATE, LDY, 2}, {0xA4, ZERO_PAGE, LDY, 3},
    {0xB4, ZERO_PAGE_X, LDY, 4}, {0xAC, ABSOLUTE, LDY, 4}, {0xBC, ABSOLUTE_X, LDY, 4}, {0x4A, ACCUMULATOR, LSR, 2},
    {0x46, ZERO_PAGE, LSR, 5}, {0x56, ZERO_PAGE_X, LSR, 6}, {0x4E, ABSOLUTE, LSR, 6}, {0x5E, ABSOLUTE_X, LSR, 7},
    {0xEA, IMPLIED, NOP, 2}, {0x09, IMMEDIATE, ORA, 2}, {0x05, ZERO_PAGE, ORA, 3}, {0x15, ZERO_PAGE_X, ORA, 4},
    {0x0D, ABSOLUTE, ORA, 4}, {0x1D, ABSOLUTE_X, ORA, 4}, {0x19, ABSOLUTE_Y, ORA, 4}, {0x01, INDIRECT_X, ORA, 6},
    {0x11, INDIRECT_Y, ORA, 5}, {0x48, IMPLIED, PHA, 3}, {0x08, IMPLIED, PHP, 3}, {0x68, IMPLIED, PLA, 4},
    {0x28, IMPLIED, PLP, 4}, {0x2A, ACCUMULATOR, ROL, 2}, {0x26, ZERO_PAGE, ROL, 5}, {0x36, ZERO_PAGE_X, ROL, 6},
    {0x2E, ABSOLUTE, ROL, 6}, {0x3E, ABSOLUTE_X, ROL, 7}, {0x6A, ACCUMULATOR, ROR, 2}, {0x66, ZERO_PAGE, ROR, 5},
    {0x76, ZERO_PAGE_X, ROR, 6}, {0x6E, ABSOLUTE, ROR, 6}, {0x7E, ABSOLUTE_X, ROR, 7}, {0x40, IMPLIED, RTI, 6},
    {0x60, IMPLIED, RTS, 6}, {0xE9, IMMEDIATE, SBC, 2}, {0xE5, ZERO_PAGE, SBC, 3}, {0xF5, ZERO_PAGE_X, SBC, 4},
    {0xED, ABSOLUTE, SBC, 4}, {0xFD, ABSOLUTE_X, SBC, 4}, {0xF9, ABSOLUTE_Y, SBC, 4}, {0xE1, INDIRECT_X, SBC, 6},
    {0xF1, INDIRECT_Y, SBC, 5}, {0x38, IMPLIED, SEC, 2}, {0xF8, IMPLIED, SED, 2}, {0x78, IMPLIED, SEI, 2},
    {0x85, ZERO_PAGE, STA, 3}, {0x95, ZERO_PAGE_X, STA, 4}, {0x8D, ABSOLUTE, STA, 4}, {0x9D, ABSOLUTE_X, STA, 5},
    {0x99, ABSOLUTE_Y, STA, 5}, {0x81, INDIRECT_X, STA, 6}, {0x91, INDIRECT_Y, STA, 6}, {0x86, ZERO_PAGE, STX, 3},
    {0x96, ZERO_PAGE_Y, STX, 4}, {0x8E, ABSOLUTE, STX, 4}, {0x84, ZERO_PAGE, STY, 3}, {0x94, ZERO_PAGE_X, STY, 4},
    {0x8C, ABSOLUTE, STY, 4}, {0xAA, IMPLIED, TAX, 2}, {0xA8, IMPLIED, TAY, 2}, {0xBA, IMPLIED, TSX, 2},
    {0x8A, IMPLIED, TXA, 2}, {0x9A, IMPLIED, TXS, 2}, {0x98, IMPLIED, TYA, 2}
};

typedef struct opcode_string_match {
    int op_num;
    char name[14];
} opcode_string_match;

opcode_string_match opcode_string_lookup[151] = {
    {0x69, "ADC"},
    {0x65, "ADC"},
    {0x75, "ADC"},
    {0x6D, "ADC"},
    {0x7D, "ADC"},
    {0x79, "ADC"},
    {0x61, "ADC"},
    {0x71, "ADC"},
    {0x29, "AND"},
    {0x25, "AND"},
    {0x35, "AND"},
    {0x2D, "AND"},
    {0x3D, "AND"},
    {0x39, "AND"},
    {0x21, "AND"},
    {0x31, "AND"},
    {0x0A, "ASL"},
    {0x06, "ASL"},
    {0x16, "ASL"},
    {0x0E, "ASL"},
    {0x1E, "ASL"},
    {0x90, "BCC"},
    {0xB0, "BCS"},
    {0xF0, "BEQ"},
    {0x24, "BIT"},
    {0x2C, "BIT"},
    {0x30, "BMI"},
    {0xD0, "BNE"},
    {0x10, "BPL"},
    {0x00, "BRK"},
    {0x50, "BVC"},
    {0x70, "BVS"},
    {0x18, "CLC"},
    {0xD8, "CLD"},
    {0x58, "CLI"},
    {0xB8, "CLV"},
    {0xC9, "CMP"},
    {0xC5, "CMP"},
    {0xD5, "CMP"},
    {0xCD, "CMP"},
    {0xDD, "CMP"},
    {0xD9, "CMP"},
    {0xC1, "CMP"},
    {0xD1, "CMP"},
    {0xE0, "CPX"},
    {0xE4, "CPX"},
    {0xEC, "CPX"},
    {0xC0, "CPY"},
    {0xC4, "CPY"},
    {0xCC, "CPY"},
    {0xC6, "DEC"},
    {0xD6, "DEC"},
    {0xCE, "DEC"},
    {0xDE, "DEC"},
    {0xCA, "DEX"},
    {0x88, "DEY"},
    {0x49, "EOR"},
    {0x45, "EOR"},
    {0x55, "EOR"},
    {0x4D, "EOR"},
    {0x5D, "EOR"},
    {0x59, "EOR"},
    {0x41, "EOR"},
    {0x51, "EOR"},
    {0xE6, "INC"},
    {0xF6, "INC"},
    {0xEE, "INC"},
    {0xFE, "INC"},
    {0xE8, "INX"},
    {0xC8, "INY"},
    {0x4C, "JMP"},
    {0x6C, "JMP"},
    {0x20, "JSR"},
    {0xA9, "LDA"},
    {0xA5, "LDA"},
    {0xB5, "LDA"},
    {0xAD, "LDA"},
    {0xBD, "LDA"},
    {0xB9, "LDA"},
    {0xA1, "LDA"},
    {0xB1, "LDA"},
    {0xA2, "LDX"},
    {0xA6, "LDX"},
    {0xB6, "LDX"},
    {0xAE, "LDX"},
    {0xBE, "LDX"},
    {0xA0, "LDY"},
    {0xA4, "LDY"},
    {0xB4, "LDY"},
    {0xAC, "LDY"},
    {0xBC, "LDY"},
    {0x4A, "LSR"},
    {0x46, "LSR"},
    {0x56, "LSR"},
    {0x4E, "LSR"},
    {0x5E, "LSR"},
    {0xEA, "NOP"},
    {0x09, "ORA"},
    {0x05, "ORA"},
    {0x15, "ORA"},
    {0x0D, "ORA"},
    {0x1D, "ORA"},
    {0x19, "ORA"},
    {0x01, "ORA"},
    {0x11, "ORA"},
    {0x48, "PHA"},
    {0x08, "PHP"},
    {0x68, "PLA"},
    {0x28, "PLP"},
    {0x2A, "ROL"},
    {0x26, "ROL"},
    {0x36, "ROL"},
    {0x2E, "ROL"},
    {0x3E, "ROL"},
    {0x6A, "ROR"},
    {0x66, "ROR"},
    {0x76, "ROR"},
    {0x6E, "ROR"},
    {0x7E, "ROR"},
    {0x40, "RTI"},
    {0x60, "RTS"},
    {0xE9, "SBC"},
    {0xE5, "SBC"},
    {0xF5, "SBC"},
    {0xED, "SBC"},
    {0xFD, "SBC"},
    {0xF9, "SBC"},
    {0xE1, "SBC"},
    {0xF1, "SBC"},
    {0x38, "SEC"},
    {0xF8, "SED"},
    {0x78, "SEI"},
    {0x85, "STA"},
    {0x95, "STA"},
    {0x8D, "STA"},
    {0x9D, "STA"},
    {0x99, "STA"},
    {0x81, "STA"},
    {0x91, "STA"},
    {0x86, "STX"},
    {0x96, "STX"},
    {0x8E, "STX"},
    {0x84, "STY"},
    {0x94, "STY"},
    {0x8C, "STY"},
    {0xAA, "TAX"},
    {0xA8, "TAY"},
    {0xBA, "TSX"},
    {0x8A, "TXA"},
    {0x9A, "TXS"},
    {0x98, "TYA"}
};

opcode_string_match address_mode_strings[13] = {
    {0, "IMPLIED"},
    {1, "ACCUMULATOR"},
    {2, "IMMEDIATE"},
    {3, "ZERO_PAGE"},
    {4, "ZERO_PAGE_X"},
    {5, "ZERO_PAGE_Y"},
    {6, "ABSOLUTE"},
    {7, "ABSOLUTE_X"},
    {8, "ABSOLUTE_Y"},
    {9, "INDIRECT"},
    {10, "INDIRECT_X"},
    {11, "INDIRECT_Y"},
    {12, "RELATIVE"}
};

char* get_address_mode_string(uint8_t address_num) {

    for(int i = 0; i < 13; i++) {
        if (address_mode_strings[i].op_num == address_num) {
            return address_mode_strings[i].name;
        }
    }
    return "?";
}

char* get_opcode_name(uint8_t op) {

    for(int i = 0; i < 151; i++) {
        if (opcode_string_lookup[i].op_num == op) {
            return opcode_string_lookup[i].name;
        }
    }
    return "NOP";
}