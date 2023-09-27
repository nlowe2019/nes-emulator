const int PATT_TABLE_0 = 0x0000;
const int PATT_TABLE_1 = 0x1000;

const int NAME_TABLE_0 = 0x2000;
const int NAME_TABLE_1 = 0x2400;
const int NAME_TABLE_2 = 0x2800;
const int NAME_TABLE_3 = 0x2C00;
const int ATTRIBUTE_OFFSET = 0x3C0;
//remainder mirror 2000-2EFF

const int IMG_PALETTE = 0x3F00;
const int SPRITE_PALETTE = 0x3F10;
//remainder mirros 3F00-3F1F

//0x4000-0x10000 MIRROS  0x0000-0x3FFF

unsigned char vram[0x3fff];
