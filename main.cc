#include <iostream>
#include <SDL2/SDL.h>
#include "chip8.hh"
#include <memory>
#include <unistd.h>

using namespace std;

struct SDLDeleter {
    void operator() (SDL_Window* window) const {
        SDL_DestroyWindow(window);
    }

    void operator() (SDL_Renderer* renderer) const {
        SDL_DestroyRenderer(renderer);
    }
};

using uniqueWindow = unique_ptr<SDL_Window, SDLDeleter>;
using uniqueRenderer = unique_ptr<SDL_Renderer, SDLDeleter>;

void usage(const char* route) {
    cerr << "Usage: " << route << "<rom_route.ch8>\n";
    exit(1);
}

int main (int argc, char* argv[]) {
    if (argc != 2) usage(argv[0]);

    CHIP8 myChip;
    myChip.load_rom(argv[1]);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "Error initializing SDL2: " << SDL_GetError() << "\n";
        return 1;
    }
    

    uniqueWindow window(SDL_CreateWindow(argv[1], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64 * 10, 32 * 10, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));
    if (!window) {
        SDL_Log("Error creating window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    uniqueRenderer renderer(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if (!renderer) {
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    if (!sdlTexture) {
        SDL_Log("Error creating texture: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    bool running = true;

    SDL_Event event;

    while (running) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: myChip.key[0x1] = 1; break;
                    case SDLK_2: myChip.key[0x2] = 1; break;
                    case SDLK_3: myChip.key[0x3] = 1; break;
                    case SDLK_4: myChip.key[0xC] = 1; break;
                    case SDLK_q: myChip.key[0x4] = 1; break;
                    case SDLK_w: myChip.key[0x5] = 1; break;
                    case SDLK_e: myChip.key[0x6] = 1; break;
                    case SDLK_r: myChip.key[0xD] = 1; break;
                    case SDLK_a: myChip.key[0x7] = 1; break;
                    case SDLK_s: myChip.key[0x8] = 1; break;
                    case SDLK_d: myChip.key[0x9] = 1; break;
                    case SDLK_f: myChip.key[0xE] = 1; break;
                    case SDLK_z: myChip.key[0xA] = 1; break;
                    case SDLK_x: myChip.key[0x0] = 1; break;
                    case SDLK_c: myChip.key[0xB] = 1; break;
                    case SDLK_v: myChip.key[0xF] = 1; break;
                }
            }

            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: myChip.key[0x1] = 0; break;
                    case SDLK_2: myChip.key[0x2] = 0; break;
                    case SDLK_3: myChip.key[0x3] = 0; break;
                    case SDLK_4: myChip.key[0xC] = 0; break;
                    case SDLK_q: myChip.key[0x4] = 0; break;
                    case SDLK_w: myChip.key[0x5] = 0; break;
                    case SDLK_e: myChip.key[0x6] = 0; break;
                    case SDLK_r: myChip.key[0xD] = 0; break;
                    case SDLK_a: myChip.key[0x7] = 0; break;
                    case SDLK_s: myChip.key[0x8] = 0; break;
                    case SDLK_d: myChip.key[0x9] = 0; break;
                    case SDLK_f: myChip.key[0xE] = 0; break;
                    case SDLK_z: myChip.key[0xA] = 0; break;
                    case SDLK_x: myChip.key[0x0] = 0; break;
                    case SDLK_c: myChip.key[0xB] = 0; break;
                    case SDLK_v: myChip.key[0xF] = 0; break;
                }
            }         
        }
        

        for (int i = 0; i < 10; ++i) {
            myChip.emulateCycle(); 
        }

        myChip.updateTimers();

        uint32_t pixels [64 * 32];

        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 32; j++) {
                int index = i + j * 64;
                if (myChip.gfx[i][j]) pixels[index] = 0xFFFFFFFF;
                else pixels[index] = 0x000000FF;
            }
        }

        SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(uint32_t));
        
        SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(renderer.get());

        SDL_RenderCopy(renderer.get(), sdlTexture, NULL, NULL);
        SDL_RenderPresent(renderer.get());
        
    }

    SDL_Quit();
    return 0;
}