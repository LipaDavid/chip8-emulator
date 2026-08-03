#include "chip8.hh"
#include <cassert>
#include <fstream>
#include <cstring>
#include <iostream>

CHIP8::CHIP8() : ram{}, V{}, stack{}, keyboard{} {
    PC = 0x200;
    I = SP = op = 0;
    delay_timer = sound_timer = 0;

    for (int i = 0; i < 16; ++i) {
        OpTable[i] = &CHIP8::OP_NULL;
    }

    OpTable[0x0] = &CHIP8::OP_0XXX;
    OpTable[0x1] = &CHIP8::OP_1NNN;
    OpTable[0x6] = &CHIP8::OP_6XNN;
    OpTable[0x7] = &CHIP8::OP_7XNN;
    OpTable[0xA] = &CHIP8::OP_ANNN;
    OpTable[0xD] = &CHIP8::OP_DXYN;
}

size_t CHIP8::load_rom(const char* route) {
    ifstream r(route, ios::binary | ios::ate);
    assert(r.is_open());

    size_t size = r.tellg();

    assert(size <= (4096 - 0x200) && "ROM is too big.");
    r.seekg(0, ios::beg);
    
    r.read(reinterpret_cast<char *>(&ram[0x200]), size);

    return size;
}

void CHIP8::emulateCycle() {
    //Fetch
    op = (ram [PC] << 8) | ram [PC + 1];
    PC += 2;

    //Decode and execute
    (this->*OpTable[(op & 0xF000) >> 12])();

    if (delay_timer > 0) --delay_timer;
    if (sound_timer > 0) --sound_timer;
    {
        if(sound_timer == 1) cout << "BEEP\n";
    }
}

void CHIP8::OP_0XXX() {
    switch(op) {
    case 0x00E0:
        //Clear grid
        memset(grid, 0, sizeof(grid));
        break;
    default:
        break;
    }
}

void CHIP8::OP_1NNN() {
    PC = op & 0x0FFF;
}

void CHIP8::OP_6XNN() {
    V[(op & 0x0F00) >> 8] = op & 0x00FF;
}

void CHIP8::OP_7XNN() {
    V[(op & 0x0F00) >> 8] += op & 0x00FF;
}

void CHIP8::OP_ANNN() {
    I = op & 0x0FFF;
}

void CHIP8::OP_DXYN() {
    V[0xF] = 0;
    uint8_t X = (op & 0x0F00) >> 8;
    uint8_t Y = (op & 0x00F0) >> 4;
    uint8_t N = (op & 0x000F);
    for (int j = V[Y]; j < V[Y] + N; j++) {
        uint8_t spriteByte = ram [I + j - V[Y]];

        for (int i = V[X] ; i < V[X] + 8; i++) {
            uint8_t tmp = grid [i % 64] [j % 32] ^ ((spriteByte >> (7 - (i - V[X])) & 0x1));

            if (grid [i % 64] [j % 32] && !tmp) V[0xF] = 1;
            grid [i % 64] [j % 32] = tmp;
        }
    }
}

void CHIP8::OP_NULL() {}