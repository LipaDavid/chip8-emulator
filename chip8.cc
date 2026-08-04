#include "chip8.hh"
#include <cassert>
#include <fstream>
#include <cstring>
#include <iostream>

CHIP8::CHIP8() : ram{}, V{}, stack{}, key{}, gfx{} {
    PC = 0x200;
    I = SP = op = 0;
    delay_timer = sound_timer = 0;

    uint8_t fontset[80] = { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    memcpy(&ram[0x0050], fontset, sizeof(fontset));

    for (int i = 0; i < 16; ++i) {
        OpTable[i] = &CHIP8::OP_NULL;
    }

    OpTable[0x0] = &CHIP8::OP_0XXX;
    OpTable[0x1] = &CHIP8::OP_1NNN;
    OpTable[0x2] = &CHIP8::OP_2NNN;
    OpTable[0x3] = &CHIP8::OP_3XNN;
    OpTable[0x4] = &CHIP8::OP_4XNN;
    OpTable[0x5] = &CHIP8::OP_5XY0;
    OpTable[0x6] = &CHIP8::OP_6XNN;
    OpTable[0x7] = &CHIP8::OP_7XNN;
    OpTable[0x8] = &CHIP8::OP_8XYX;
    OpTable[0x9] = &CHIP8::OP_9XY0;
    OpTable[0xA] = &CHIP8::OP_ANNN;
    OpTable[0xB] = &CHIP8::OP_BNNN;
    OpTable[0xC] = &CHIP8::OP_CXNN;
    OpTable[0xD] = &CHIP8::OP_DXYN;
    OpTable[0xE] = &CHIP8::OP_EXXX;
    OpTable[0xF] = &CHIP8::OP_FXXX;
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
// Blindaje del PC
    if (PC >= 4094) return; 

    // Fetch
    op = (ram[PC] << 8) | ram[PC + 1];
    
    // --- CHIVATO PARA DEBUG ---
    // Imprime qué está haciendo la CPU en formato hexadecimal
    std::cout << "PC: 0x" << std::hex << PC << " | Opcode: 0x" << op << std::endl;
    
    PC += 2;

    // Decode and execute
    (this->*OpTable[(op & 0xF000) >> 12])();
}

void CHIP8::updateTimers() {
    if (delay_timer > 0) --delay_timer;
    if (sound_timer > 0) --sound_timer;
    if (sound_timer == 1) cout << "BEEP\n";    
}

void CHIP8::OP_0XXX() {
    switch(op) {
        case 0x00E0:
            //Clear gfx
            memset(gfx, 0, sizeof(gfx));
            break;

        case 0x00EE:
            SP--;
            PC = stack[SP];
            break;
            
        default:
            break;
    }
}

void CHIP8::OP_1NNN() {
    PC = op & 0x0FFF;
}

void CHIP8::OP_2NNN() {
    stack[SP] = PC;
    SP++;

    PC = op & 0x0FFF;

}

void CHIP8::OP_3XNN() {
    if (V[(op & 0x0F00) >> 8] == (op & 0x00FF)) PC += 2;
}

void CHIP8::OP_4XNN() {
    if (V[(op & 0x0F00) >> 8] != (op & 0x00FF)) PC += 2;
}

void CHIP8::OP_5XY0() {
    if (V[(op & 0x0F00) >> 8] == V[(op & 0x00F0) >> 4]) PC += 2;
}

void CHIP8::OP_6XNN() {
    V[(op & 0x0F00) >> 8] = op & 0x00FF;
}

void CHIP8::OP_7XNN() {
    V[(op & 0x0F00) >> 8] += op & 0x00FF;
}

void CHIP8::OP_8XYX() {
    uint8_t X = (op & 0x0F00) >> 8;
    uint8_t Y = (op & 0x00F0) >> 4;
    switch (op & 0x000F) {
        case 0:
            V[X] = V[Y];
            break;

        case 1:
            V[X] |= V[Y];
            break;

        case 2:
            V[X] &= V[Y];
            break;

        case 3:
            V[X] ^= V[Y];
            break;

        case 4: {
            uint16_t sum = V[X] + V[Y];
            V[0xF] = (sum > 255) ? 1 : 0;
            V[X] = sum & 0xFF;
            break;
        }

        case 5:
            V[0xF] = (V[X] >= V[Y]) ? 1 : 0;
            V[X] -= V[Y];
            break;

        case 6:
            V[0xF] = V[X] & 0x1;
            V[X] >>= 1;
            break;

        case 7:
            V[0xF] = (V[X] <= V[Y]) ? 1 : 0;
            V[X] = V[Y] - V[X];
            break;

        case 0x000E:
            V[0xF] = (V[X] & 0x80) >> 7;
            V[X] <<= 1;
            break;

        default:
            break;
    }
}

void CHIP8::OP_9XY0() {
    if (V[(op & 0x0F00) >> 8] != V[(op & 0x00F0) >> 4]) PC += 2;
}

void CHIP8::OP_ANNN() {
    I = op & 0x0FFF;
}

void CHIP8::OP_BNNN() {
    PC = V[0] + (op & 0x0FFF);
}

void CHIP8::OP_CXNN() {
    V[(op & 0x0F00) >> 8] = rand() & (op & 0xFF);
}

void CHIP8::OP_DXYN() {
    
    uint8_t X = (op & 0x0F00) >> 8;
    uint8_t Y = (op & 0x00F0) >> 4;
    uint8_t N = (op & 0x000F);

    V[0xF] = 0;

    for (int j = V[Y]; j < V[Y] + N; j++) {
        uint8_t spriteByte = ram [I + j - V[Y]];

        for (int i = V[X] ; i < V[X] + 8; i++) {
            uint8_t tmp = gfx [i % 64] [j % 32] ^ ((spriteByte >> (7 - (i - V[X])) & 0x1));

            if (gfx [i % 64] [j % 32] && !tmp) V[0xF] = 1;
            gfx [i % 64] [j % 32] = tmp;
        }
    }
}

void CHIP8::OP_EXXX() {
    switch(op & 0x00FF) {
        case 0x009E:
            if (key[V[(op & 0x0F00) >> 8]] != 0) PC += 2;
            break;

        case 0x00A1:
            if (key[V[(op & 0x0F00) >> 8]] == 0) PC += 2;
            break;  

        default:
            break;
    }
}

void CHIP8::OP_FXXX() {
    uint8_t X = (op & 0x0F00) >> 8;

    switch(op & 0x00FF) {
        case 0x0007:
            V[X] = delay_timer;
            break;

        case 0x000A: {
            bool key_pressed = false;

            for (int i = 0; i < 16; i++) {
                if (key[i]) {
                    V[X] = i;
                    key_pressed = true;
                    break;
                }
            }

            if (not key_pressed) PC -= 2;
            break;
        }

        case 0x0015:
            delay_timer = V[X];
            break;
        
        case 0x0018:
            sound_timer = V[X];
            break;
        
        case 0x001E:
            I += V[X];
            break;
        
        case 0x0029:
            I = 0x0050 + (V[X] * 5);
            break;
        
        case 0x0033:
            ram[I] = V[X] / 100; 
            ram[I + 1] = (V[X] / 10) % 10;
            ram[I + 2] = V[X] % 10;
            break;

        case 0x0055:
            for (int i = 0; i <= X; i++) ram[I + i] = V[i];
            break;

        case 0x0065:
            for (int i = 0; i <= X; i++) V[i] = ram[I + i];
            break;

        default:
            break;
    }
}

void CHIP8::OP_NULL() {}