#include "apu.h"
#include <sys/types.h>
#include <stdint.h>

uint8_t apu_reg[0x18];
// register numbers:
const int PLS1_ENVELOPE = 0x0;
const int PLS1_SWEEP = 0x1;
const int PLS1_TIMER = 0x2;
const int PLS1_LENGTH_COUNTER = 0x3;

const int PLS2_ENVELOPE = 0x4;
const int PLS2_SWEEP = 0x5;
const int PLS2_TIMER = 0x6;
const int PLS2_LENGTH_COUNTER = 0x7;

const int TRI_LINEAR_COUNTER =0x8; 
const int TRI_TIMER = 0xa;
const int TRI_LENGTH_COUNTER = 0xb;

const int NOISE_ENVELOPE = 0xc;
const int NOISE_PERIOD = 0xe;
const int NOISE_LENGTH_COUNTER = 0xf;

const int DMC_FREQ = 0x10;
const int DMC_COUNTER = 0x11;
const int DMC_SAMPLE_ADDR = 0x12;
const int DMC_SAMPLE_LENGTH = 0x13;

const int APU_STATUS = 0x15;
const int FRAME_COUNTER = 0x17;

uint8_t apu_read(uint16_t addr) {
    addr %= 0x4000;

    if(addr == APU_STATUS)
        return apu_reg[APU_STATUS];
    else
        return 0;
}

void apu_write(uint16_t addr, uint8_t data) {

    addr %= 0x4000;

    apu_reg[addr] = data;
}