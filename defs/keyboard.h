#pragma once
#include <cstdint>
#include <SDL2/SDL_events.h>

// classe que cuida das teclas do chip8
class Keyboard {
public:
    Keyboard(); // construtor, inicia as teclas como false

    // trata um evento do sdl (keydown ou keyup) e marca a tecla correspondente
    void handleEvent(const SDL_Event &e);

    // verifica se uma tecla do chip8 (0x0 a 0xF) esta pressionada
    bool isPressed(uint8_t chip8_key) const;

    // bloqueia ate alguma tecla valida ser pressionada (usado no opcode fx0a)
    int waitForKey();

private:
    bool keys[16]; // cada posicao representa uma tecla (true = pressionada)
};
