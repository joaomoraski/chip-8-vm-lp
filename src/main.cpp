#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <chrono>

#include "../defs/chip8.h"
#include "../defs/display.h"
#include "../defs/keyboard.h"
#include "../defs/defs.h"

// struct pra guardar as configs que vem da linha de comando
struct Config {
    int scale = DEFAULT_SCALE;       // tamanho da tela
    int clock_hz = DEFAULT_CLOCK_HZ; // velocidade da cpu
    std::string rom;                 // caminho da rom
    int color_r = 255;               // cor padrao (branco)
    int color_g = 255;
    int color_b = 255;
};

// mostra as instrucoes pro usuario
static void print_help(const char *prog) {
    std::printf(
        "Uso: %s [opcoes] <rom.ch8>\n"
        "Opcoes:\n"
        "  --scale <n>        escala da janela (padrao %d)\n"
        "  --clock <hz>       velocidade da cpu (padrao %d)\n"
        "  --color <r> <g> <b> cor dos pixels (0-255 cada, padrao branco)\n"
        "  --help             mostra essa mensagem\n",
        prog, DEFAULT_SCALE, DEFAULT_CLOCK_HZ);
}

// le os argumentos do terminal
static bool parse_args(int argc, char **argv, Config &cfg) {
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return false;

        } else if (std::strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            cfg.scale = std::atoi(argv[++i]);

        } else if (std::strcmp(argv[i], "--clock") == 0 && i + 1 < argc) {
            cfg.clock_hz = std::atoi(argv[++i]);

        } else if (std::strcmp(argv[i], "--color") == 0 && i + 3 < argc) {
            // nova opcao pra definir cor pelo terminal
            cfg.color_r = std::atoi(argv[++i]);
            cfg.color_g = std::atoi(argv[++i]);
            cfg.color_b = std::atoi(argv[++i]);

        } else if (argv[i][0] == '-') {
            std::fprintf(stderr, "Opcao desconhecida: %s\n", argv[i]);
            return false;

        } else {
            // o que sobrar é o caminho da rom
            cfg.rom = argv[i];
        }
    }

    if (cfg.rom.empty()) {
        print_help(argv[0]);
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    Config cfg;
    if (!parse_args(argc, argv, cfg)) return 1; // se der erro nos argumentos, sai

    // inicia o sdl (video, audio e timer)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // cria o display (tela) e teclado
    Display display;
    if (!display.init(cfg.scale)) return 1;
    Keyboard keyboard;

    // cria a vm e carrega a rom
    Chip8 vm;
    vm.initialize();
    if (!vm.loadROM(cfg.rom)) {
        std::fprintf(stderr, "Falha ao carregar ROM: %s\n", cfg.rom.c_str());
        return 1;
    }

    // define os intervalos de tempo (cpu, frame e timer)
    const double cpu_dt_ms = 1000.0 / (double) cfg.clock_hz;
    const double frame_dt_ms = 1000.0 / 60.0;
    const double timer_dt_ms = 1000.0 / 60.0;

    auto last_cpu = std::chrono::high_resolution_clock::now();
    auto last_frame = last_cpu;
    auto last_timer = last_cpu;

    bool running = true;
    SDL_Event e;

    // loop principal do programa
    while (running && display.isOpen()) {
        // trata os eventos (teclado, sair, etc)
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            keyboard.handleEvent(e);
        }

        // calcula quanto tempo passou pra saber se roda outro ciclo da cpu
        auto now = std::chrono::high_resolution_clock::now();
        double ms_since_cpu = std::chrono::duration<double, std::milli>(now - last_cpu).count();
        while (ms_since_cpu >= cpu_dt_ms) {
            vm.emulateCycle(keyboard, display);
            last_cpu += std::chrono::milliseconds((int) cpu_dt_ms);
            now = std::chrono::high_resolution_clock::now();
            ms_since_cpu = std::chrono::duration<double, std::milli>(now - last_cpu).count();
        }

        // atualiza timers a cada 1/60s
        double ms_since_timer = std::chrono::duration<double, std::milli>(now - last_timer).count();
        if (ms_since_timer >= timer_dt_ms) {
            vm.tickTimers();
            last_timer = now;
        }

        // atualiza o frame da tela a cada 1/60s
        double ms_since_frame = std::chrono::duration<double, std::milli>(now - last_frame).count();
        if (ms_since_frame >= frame_dt_ms) {
            // aqui passa a cor escolhida pro draw
            // precisa ajustar a funcao draw no display pra aceitar rgb
            display.draw(vm.video(), cfg.color_r, cfg.color_g, cfg.color_b);
            last_frame = now;
        }

        // pequena pausa pra nao travar o sistema
        SDL_Delay(1);
    }

    // fecha tudo
    display.shutdown();
    SDL_Quit();
    return 0;
}
