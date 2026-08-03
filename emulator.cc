#include "chip8.hh"
#include <iomanip>
#include <iostream>

CHIP8 myChip;

using namespace std;

int main () {
    size_t rom_size = myChip.load_rom("./IBM_Logo.ch8");

    for (int i = 0; i < 50; i++) myChip.emulateCycle();

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (myChip.grid [x] [y]) cout << "██";
            else cout << "  ";
        }
        cout << "\n";
    }
}
