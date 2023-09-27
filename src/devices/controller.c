#include "../memory/mem.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

uint8_t controller_reg_1 = 0;
uint8_t controller_reg_2 = 0;

uint8_t strobe = false;

// A -> B -> SELECT -> START -> UP -> DOWN -> LEFT -> RIGHT
int counter = 0;
uint8_t buttons[8] = {0,0,0,0,0,0,0,0};

uint8_t controller_read(uint16_t addr) {

    if(addr == 0x4016 || 1) {

        if(strobe)
            return buttons[0];

        else {
            if(counter > 7) {

                return 1;
            }
            counter++;
            //printf("button[%d] : %d\n", counter-1, buttons[counter-1]);
            return buttons[counter-1];
        }
    }
    else {
        //printf("button: %d = %d\n", counter-1, buttons[counter-1]);
        return controller_reg_2;
    }
}

void controller_write(uint8_t data) { 
    
    data &= 1;

    if(data == 0) {
        strobe = false;
        counter = 0;
    }
    else
        strobe = true;
}

void button_down(int b) {

    buttons[b] = 1;
    controller_reg_1 |= (1 << b);
}

void button_up(int b) {

    buttons[b] = 0;
    controller_reg_1 &= ~(1 << b);
}

void button_reset() {

    for(int i = 0; i < 8; i++) {
        buttons[i] = 0;
    }
}