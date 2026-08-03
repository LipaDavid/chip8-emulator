#include <cstdint>
#include <cassert>
#include <fstream>
#include <cstring>
#include <iostream>

using namespace std;

class CHIP8 {
    public:
        uint8_t ram[4096] = {};

        uint8_t V[16] = {};

        uint16_t I = 0;

        uint16_t PC = 0x200;

        uint8_t grid [64] [32];

        uint8_t delay_timer = 0;
        uint8_t sound_timer = 0;

        uint16_t stack[16] = {};
        uint16_t sp = 0;

        uint8_t keyboard[16] = {};
        
        size_t load_rom(const char* route) {
            ifstream r(route, ios::binary | ios::ate);
            assert(r.is_open());

            size_t size = r.tellg();

            assert(size <= (4096 - 0x200) && "ROM is too big.");
            r.seekg(0, ios::beg);
            
            r.read(reinterpret_cast<char *>(&ram[0x200]), size);

            return size;
        }

        void emulateCycle() {
            //Fetch
            uint16_t op = (ram [PC] << 8) | ram [PC + 1];
            PC += 2;

            //Decode and execute

            decode_and_execute(op);

            if (delay_timer > 0) --delay_timer;
            if (sound_timer > 0) --sound_timer;
            {
                if(sound_timer == 1) cout << "BEEP\n";
            }
        }

    private:

        void decode_and_execute(uint16_t op) {
            switch(op & 0xF000) {
                case 0x0000:
                    switch(op) {
                        case 0x00E0:
                            //Clear grid
                            memset(grid, 0, sizeof(grid));
                            break;
                        default:
                            break;


                    }
                    break;

                case 0x1000:
                    PC = op & 0x0FFF;
                    break;

                case 0x6000:
                    V[(op & 0x0F00) >> 8] = op & 0x00FF;
                    break;

                case 0x7000:
                    V[(op & 0x0F00) >> 8] += op & 0x00FF;
                    break;

                case 0xA000:
                    I = op & 0x0FFF;
                    break;

                case 0xB000:
                    break;

                case 0xC000:
                    break;

                case 0xD000:
                    draw((op & 0x0F00) >> 8, (op & 0x00F0) >> 4, (op & 0x000F));
                    break;

                case 0xE000:
                    break;

                case 0xF000:
                    break;

                default:
                    break;
            }
        }

        void draw(uint8_t X, uint8_t Y, uint8_t N) {
            V[0xF] = 0;

            for (int j = V[Y]; j < V[Y] + N; j++) {
                uint8_t spriteByte = ram [I + j - V[Y]];

                for (int i = V[X] ; i < V[X] + 8; i++) {
                    uint8_t tmp = grid [i % 64] [j % 32] ^ ((spriteByte >> (7 - (i - V[X])) & 0x1));

                    if (grid [i % 64] [j % 32] && !tmp) V[0xF] = 1;
                    grid [i % 64] [j % 32] = tmp;
                }
            }
        }
};