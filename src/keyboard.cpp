#include "keyboard.h"
#include <SDL2/SDL.h>
#include <map>

Keyboard::Keyboard() {
    for (int i = 0; i < 16; ++i) keys[i] = false;
}

static uint8_t mapSDLKeyToChip8(SDL_Keycode kc) {
    // Suggested mapping in the spec:
    // CHIP-8:  1 2 3 C    -> Physical: 1 2 3 4
    //          4 5 6 D    ->            Q W E R
    //          7 8 9 E    ->            A S D F
    //          A 0 B F    ->            Z X C V
    switch (kc) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return 0xFF;
    }
}

void Keyboard::handleEvent(const SDL_Event &e) {
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        uint8_t k = mapSDLKeyToChip8(e.key.keysym.sym);
        if (k != 0xFF) {
            keys[k] = (e.type == SDL_KEYDOWN);
        }
    }
}

bool Keyboard::isPressed(uint8_t chip8_key) const {
    if (chip8_key > 0xF) return false;
    return keys[chip8_key];
}

int Keyboard::waitForKey() {
    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                uint8_t k = mapSDLKeyToChip8(e.key.keysym.sym);
                if (k != 0xFF) return (int) k;
            }
        }
        SDL_Delay(1);
    }
}
