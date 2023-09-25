#include <sys/types.h>

// addressing modes
enum ADDRESS_MODE
{
    IMPLIED,
    ACCUMULATOR,
    IMMEDIATE,
    ZERO_PAGE,
    ZERO_PAGE_X,
    ZERO_PAGE_Y,
    ABSOLUTE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    INDIRECT,
    INDIRECT_X,
    INDIRECT_Y,
    RELATIVE
};

enum INSTRUCTION
{
    ADC, AND, ASL, BCC, BCS, BEQ,
    BIT, BMI, BNE, BPL, BRK, BVC,
    BVS, CLC, CLD, CLI, CLV, CMP,
    CPX, CPY, DEC, DEX, DEY, EOR,
    INC, INX, INY, JMP, JSR, LDA,
    LDX, LDY, LSR, NOP, ORA, PHA,
    PHP, PLA, PLP, ROL, ROR, RTI,
    RTS, SBC, SEC, SED, SEI, STA,
    STX, STY, TAX, TAY, TSX, TXA,
    TXS, TYA
};

enum FLAG 
{
    FLAG_CARRY = 1 << 0,
    FLAG_ZERO = 1 << 1,
    FLAG_INTERRUPT = 1 << 2,
    FLAG_DECIMAL = 1 << 3,
    FLAG_BREAK = 1 << 4,
    FLAG_5 = 1 << 5,
    FLAG_OVERFLOW = 1 << 6,
    FLAG_NEGATIVE = 1 << 7,
};

typedef struct opcode {
    unsigned char OPCODE;
    enum ADDRESS_MODE MODE;
    enum INSTRUCTION INSTRUCTION;
    int cycles;
} opcode;

extern opcode opcode_table[256];

unsigned char FetchInstruction(void);
opcode OpcodeLookup(unsigned char op);
int ExecuteInstruction(opcode opcode);
u_int16_t GetAddress(enum ADDRESS_MODE mode, int* page_cross);

void SetFlag(int flag, bool condition);

int adc6502(enum ADDRESS_MODE mode, u_int16_t addr);
int and6502(enum ADDRESS_MODE mode, u_int16_t addr);
int asl6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bcc6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bcs6502(enum ADDRESS_MODE mode, u_int16_t addr);
int beq6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bit6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bmi6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bne6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bpl6502(enum ADDRESS_MODE mode, u_int16_t addr);
int brk6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bvc6502(enum ADDRESS_MODE mode, u_int16_t addr);
int bvs6502(enum ADDRESS_MODE mode, u_int16_t addr);
int clv6502(enum ADDRESS_MODE mode, u_int16_t addr);
int cmp6502(enum ADDRESS_MODE mode, u_int16_t addr);
int cpx6502(enum ADDRESS_MODE mode, u_int16_t addr);
int cpy6502(enum ADDRESS_MODE mode, u_int16_t addr);
int dec6502(enum ADDRESS_MODE mode, u_int16_t addr);
int dex6502(enum ADDRESS_MODE mode, u_int16_t addr);
int dey6502(enum ADDRESS_MODE mode, u_int16_t addr);
int eor6502(enum ADDRESS_MODE mode, u_int16_t addr);
int inc6502(enum ADDRESS_MODE mode, u_int16_t addr);
int inx6502(enum ADDRESS_MODE mode, u_int16_t addr);
int iny6502(enum ADDRESS_MODE mode, u_int16_t addr);
int jmp6502(enum ADDRESS_MODE mode, u_int16_t addr);
int jsr6502(enum ADDRESS_MODE mode, u_int16_t addr);
int lda6502(enum ADDRESS_MODE mode, u_int16_t addr);
int ldx6502(enum ADDRESS_MODE mode, u_int16_t addr);
int ldy6502(enum ADDRESS_MODE mode, u_int16_t addr);
int lsr6502(enum ADDRESS_MODE mode, u_int16_t addr);
int nop6502(enum ADDRESS_MODE mode, u_int16_t addr);
int ora6502(enum ADDRESS_MODE mode, u_int16_t addr);
int pha6502(enum ADDRESS_MODE mode, u_int16_t addr);
int php6502(enum ADDRESS_MODE mode, u_int16_t addr);
int pla6502(enum ADDRESS_MODE mode, u_int16_t addr);
int plp6502(enum ADDRESS_MODE mode, u_int16_t addr);
int rol6502(enum ADDRESS_MODE mode, u_int16_t addr);
int ror6502(enum ADDRESS_MODE mode, u_int16_t addr);
int rti6502(enum ADDRESS_MODE mode, u_int16_t addr);
int rts6502(enum ADDRESS_MODE mode, u_int16_t addr);
int sbc6502(enum ADDRESS_MODE mode, u_int16_t addr);
int sta6502(enum ADDRESS_MODE mode, u_int16_t addr);
int stx6502(enum ADDRESS_MODE mode, u_int16_t addr);
int sty6502(enum ADDRESS_MODE mode, u_int16_t addr);
int tax6502(enum ADDRESS_MODE mode, u_int16_t addr);
int tay6502(enum ADDRESS_MODE mode, u_int16_t addr);
int tsx6502(enum ADDRESS_MODE mode, u_int16_t addr);
int txa6502(enum ADDRESS_MODE mode, u_int16_t addr);
int txs6502(enum ADDRESS_MODE mode, u_int16_t addr);
int tya6502(enum ADDRESS_MODE mode, u_int16_t addr);
int sec6502(enum ADDRESS_MODE mode, u_int16_t addr);
int sei6502(enum ADDRESS_MODE mode, u_int16_t addr);
int sed6502(enum ADDRESS_MODE mode, u_int16_t addr);
int clc6502(enum ADDRESS_MODE mode, u_int16_t addr);
int cli6502(enum ADDRESS_MODE mode, u_int16_t addr);
int cld6502(enum ADDRESS_MODE mode, u_int16_t addr);

void Init_CPU();
int Update_CPU();

void StackPush(unsigned char value);
unsigned char StackPop(void);

extern unsigned char accumulator;
extern unsigned char index_x;
extern unsigned char index_y;
extern unsigned char status;
extern u_int16_t pc;
extern unsigned char sp;

char * get_opcode_name(unsigned char op);
char * get_address_mode_string(unsigned char address_num);