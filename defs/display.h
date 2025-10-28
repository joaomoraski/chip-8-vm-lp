#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "defs.h"

// classe que representa a tela do chip8
class Display {
public:
    Display(); // construtor
    ~Display(); // destrutor (fecha janela)

    // cria a janela e o renderizador do sdl com o tamanho certo
    bool init(int scale);

    // desenha os pixels na tela baseado no buffer da vm
    void draw(const uint8_t* framebuffer, int r_color = 255, int g_color = 255, int b_color = 255);

    // limpa a tela
    void clear();

    // fecha a janela e destroi os recursos
    void shutdown();

    // retorna se a janela esta aberta ou nao
    bool isOpen() const { return window != nullptr; }

private:
    SDL_Window *window; // janela do sdl
    SDL_Renderer *renderer; // renderizador do sdl
    int scale; // escala do tamanho da tela
};
