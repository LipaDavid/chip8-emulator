#include <cstdint>

using namespace std;

class CHIP8 {
    public:
        CHIP8();

        uint8_t ram[4096], V[16], keyboard[16];

        uint16_t I, PC, SP, op;

        uint8_t grid [64] [32], stack[16];

        uint8_t delay_timer, sound_timer;

        size_t load_rom(const char* route);

        void emulateCycle();

    private:

        using Instruction = void (CHIP8::*)();

        Instruction OpTable[16];

        void OP_0XXX();
        void OP_1NNN();
        void OP_6XNN();
        void OP_7XNN();
        void OP_ANNN();
        void OP_DXYN();
        void OP_NULL();
};