#include <cstdint>

using namespace std;

class CHIP8 {
    public:
        CHIP8();

        uint8_t ram[4096], V[16], key[16], gfx[64] [32];

        uint16_t I, PC, SP, op, delay_timer, sound_timer, stack[16];

        size_t load_rom(const char* route);

        void emulateCycle();

        void updateTimers();

    private:

        using Instruction = void (CHIP8::*)();

        Instruction OpTable[16];

        void OP_0XXX();
        void OP_1NNN();
        void OP_2NNN();
        void OP_3XNN();
        void OP_4XNN();
        void OP_5XY0();
        void OP_6XNN();
        void OP_7XNN();
        void OP_8XYX();
        void OP_9XY0();
        void OP_ANNN();
        void OP_BNNN();
        void OP_CXNN();
        void OP_DXYN();
        void OP_EXXX();
        void OP_FXXX();
        void OP_NULL();
};