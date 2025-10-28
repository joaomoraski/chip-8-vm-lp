#include "display.h"
#include <cstdio>

// construtor, ja deixa os ponteiros nulos e a escala padrao
Display::Display() : window(nullptr), renderer(nullptr), scale(10) {
}

// destrutor, chama shutdown pra fechar corretamente
Display::~Display() {
    shutdown();
}

// inicializa o display (janela e renderizador do sdl)
bool Display::init(int scl) {
    scale = scl; // define a escala passada por parametro
    // inicia o modulo de video do sdl
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL video init error: %s\n", SDL_GetError());
        return false;
    }

    // cria a janela do chip-8 com o tamanho certo
    window = SDL_CreateWindow("CHIP-8",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              CHIP8_WIDTH * scale, CHIP8_HEIGHT * scale,
                              0);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        return false;
    }

    // cria o renderizador
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        return false;
    }

    // define o modo de mistura
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // limpa a tela no inicio
    clear();
    return true;
}

// limpa a tela
void Display::clear() {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // cor preta
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

// desenha o framebuffer (o que vem da vm chip8)
void Display::draw(const uint8_t *framebuffer, int r_color, int g_color, int b_color) {
    if (!renderer) return;

    // primeiro limpa a tela toda (preto)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // define a cor passada pelo usuario pra desenhar os pixels
    SDL_Rect r;
    r.w = scale;
    r.h = scale;
    SDL_SetRenderDrawColor(renderer, r_color, g_color, b_color, 255);

    // percorre a tela do chip8 (64x32)
    for (int y = 0; y < CHIP8_HEIGHT; ++y) {
        for (int x = 0; x < CHIP8_WIDTH; ++x) {
            // se o pixel tiver 1, desenha um quadradinho
            if (framebuffer[y * CHIP8_WIDTH + x]) {
                r.x = x * scale;
                r.y = y * scale;
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }

    // mostra o que foi desenhado na janela
    SDL_RenderPresent(renderer);
}

// fecha a janela e libera memoria
void Display::shutdown() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO); // fecha o modulo de video do sdl
}
