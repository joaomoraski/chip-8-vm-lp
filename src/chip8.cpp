#include "chip8.h"
#include "keyboard.h"
#include "display.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <random>

// construtor da vm, chama initialize pra deixar tudo zerado
Chip8::Chip8() {
    initialize(DEFAULT_PC_START);
}

void Chip8::initialize(uint16_t start_pc) {
    // seta o pc pro endereco inicial do programa (0x200)
    PC = start_pc;
    // zera o registrador de endereco
    I = 0;
    // zera o ponteiro da pilha
    SP = 0;
    // zera o timer
    delay_timer = 0;
    // zera toda a memoria e os registradores
    std::memset(memory, 0, sizeof(memory));
    std::memset(V, 0, sizeof(V));
    std::memset(stack, 0, sizeof(stack));
    std::memset(DISPLAY, 0, sizeof(DISPLAY));

    // carrega o conjunto de fontes padrao (sprites dos numeros 0-F)
    // esses bytes sao desenhados quando o programa pede pra mostrar numeros
    static const uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80
    };
    for (int i = 0; i < 80; ++i) memory[i] = fontset[i];
}

// le o arquivo da rom e coloca na memoria a partir do endereco 0x200
bool Chip8::loadROM(const std::string &path, uint16_t load_addr) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        return false;
    }

    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);

    if (load_addr + size > 4096) {
        return false;
    }
    f.read(reinterpret_cast<char *>(&memory[load_addr]), size);
    return true;
}

// reduz os timers em 1
void Chip8::tickTimers() {
    if (delay_timer > 0) --delay_timer;
    if (sound_timer > 0) --sound_timer;
}

// 00E0 - limpa a tela
void Chip8::op_00E0() {
    std::memset(DISPLAY, 0, sizeof(DISPLAY));
}

// 00EE - retorna de uma subrotina (volta da pilha)
void Chip8::op_00EE() {
    if (SP == 0) {
        unknown(0x00EE);
        return;
    }
    --SP;
    PC = stack[SP];
}

// 1NNN - pula pra endereco NNN
void Chip8::op_1NNN(uint16_t opcode) { PC = opcode & 0x0FFF; }

// 2NNN - chama subrotina (empilha o pc e pula)
void Chip8::op_2NNN(uint16_t opcode) {
    if (SP >= 16) {
        unknown(opcode);
        return;
    }
    stack[SP++] = PC;
    PC = opcode & 0x0FFF;
}

// 3XNN - pula a proxima instrucao se vx == nn
void Chip8::op_3XNN(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    if (V[x] == nn) PC += 2;
}

// 4XNN - pula se vx != nn
void Chip8::op_4XNN(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    if (V[x] != nn) PC += 2;
}

// 5XY0 - compara dois registradores
void Chip8::op_5XY0(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    if ((opcode & 0x000F) == 0x0) {
        if (V[x] == V[y]) PC += 2;
    } else {
        unknown(opcode);
    }
}

// 6XNN - coloca valor direto em vx
void Chip8::op_6XNN(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = nn;
}

// 7XNN - soma valor em vx (sem carry)
void Chip8::op_7XNN(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = static_cast<uint8_t>(V[x] + nn);
}

// 8XY_ - operacoes entre registradores
void Chip8::op_8XY_(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    switch (opcode & 0x000F) {
        case 0x0: V[x] = V[y];
            break; // copia
        case 0x1: V[x] |= V[y];
            break; // or
        case 0x2: V[x] &= V[y];
            break; // and
        case 0x3: V[x] ^= V[y];
            break; // xor
        case 0x4: {
            // soma com carry
            uint16_t sum = V[x] + V[y];
            V[0xF] = (sum > 0xFF);
            V[x] = static_cast<uint8_t>(sum);
            break;
        }
        case 0x5: {
            // subtrai
            V[0xF] = (V[x] >= V[y]);
            V[x] -= V[y];
            break;
        }
        case 0x6: {
            // shift right
            V[0xF] = V[x] & 0x1;
            V[x] >>= 1;
            break;
        }
        case 0x7: {
            // vy - vx
            V[0xF] = (V[y] >= V[x]);
            V[x] = V[y] - V[x];
            break;
        }
        case 0xE: {
            // shift left
            V[0xF] = (V[x] & 0x80) != 0;
            V[x] <<= 1;
            break;
        }
        default: unknown(opcode);
    }
}

// 9XY0 - pula se vx != vy
void Chip8::op_9XY0(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    if ((opcode & 0x000F) == 0x0) {
        if (V[x] != V[y]) PC += 2;
    } else {
        unknown(opcode);
    }
}

// ANNN - coloca endereco em I
void Chip8::op_ANNN(uint16_t opcode) { I = opcode & 0x0FFF; }

// BNNN - pula pra nnn + v0
void Chip8::op_BNNN(uint16_t opcode) { PC = (opcode & 0x0FFF) + V[0]; }

// CXNN - gera numero aleatorio & nn
void Chip8::op_CXNN(uint16_t opcode) {
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, 255);
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = static_cast<uint8_t>(dist(rng) & nn);
}

// DXYN - desenha sprite (n linhas) na tela
void Chip8::op_DXYN(uint16_t opcode, Display &disp) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = (opcode & 0x000F);
    uint8_t X = V[x] % CHIP8_WIDTH;
    uint8_t Y = V[y] % CHIP8_HEIGHT;
    V[0xF] = 0;

    // percorre as linhas do sprite e desenha na tela
    for (int row = 0; row < n; ++row) {
        uint8_t spriteByte = memory[I + row];
        for (int col = 0; col < 8; ++col) {
            if ((spriteByte & (0x80 >> col)) != 0) {
                int px = (X + col) % CHIP8_WIDTH;
                int py = (Y + row) % CHIP8_HEIGHT;
                int idx = py * CHIP8_WIDTH + px;
                if (DISPLAY[idx] == 1) V[0xF] = 1; // colisao
                DISPLAY[idx] ^= 1; // alterna pixel (xor)
            }
        }
    }
}

// EX__ - instrucoes de teclado
void Chip8::op_EX__(uint16_t opcode, Keyboard &kb) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    switch (opcode & 0x00FF) {
        case 0x9E: if (kb.isPressed(V[x])) PC += 2;
            break; // pula se tecla ta pressionada
        case 0xA1: if (!kb.isPressed(V[x])) PC += 2;
            break; // pula se tecla nao ta pressionada
        default: unknown(opcode);
    }
}

// FX__ - operacoes de timer, memoria, etc
void Chip8::op_FX__(uint16_t opcode, Keyboard &kb, Display &disp) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    switch (opcode & 0x00FF) {
        case 0x07: V[x] = delay_timer;
            break; // le o delay timer
        case 0x0A: {
            int k = kb.waitForKey();
            V[x] = k;
            break;
        } // espera tecla
        case 0x15: delay_timer = V[x];
            break; // seta delay timer
        case 0x18: sound_timer = V[x];
            break; // seta sound timer (sem som, so pra nao dar erro)
        case 0x1E: I += V[x];
            break; // soma v[x] em I
        case 0x29: I = V[x] * 5;
            break; // pega endereco da fonte do digito
        case 0x33: {
            // bcd (conversao pra decimal)
            uint8_t val = V[x];
            memory[I] = val / 100;
            memory[I + 1] = (val / 10) % 10;
            memory[I + 2] = val % 10;
            break;
        }
        case 0x55: for (int i = 0; i <= x; ++i) memory[I + i] = V[i];
            break; // salva registradores
        case 0x65: for (int i = 0; i <= x; ++i) V[i] = memory[I + i];
            break; // carrega registradores
        default: unknown(opcode);
    }
}

// funcao pra opcode invalido (so imprime erro)
void Chip8::unknown(uint16_t opcode) const {
    std::fprintf(stderr, "Unknown/unsupported opcode: 0x%04X at PC=0x%04X\n", opcode, PC);
}

// executa 1 ciclo da cpu (busca, decodifica, executa)
void Chip8::emulateCycle(Keyboard &kb, Display &disp) {
    // pega 2 bytes da memoria e forma o opcode
    uint16_t opcode = (memory[PC] << 8) | memory[PC + 1];
    PC += 2;

    // separa o grupo
    uint8_t group = (opcode & 0xF000) >> 12;

    // escolhe a operacao pelo grupo
    switch (group) {
        case 0x0:
            switch (opcode & 0x00FF) {
                case 0xE0: op_00E0();
                    break;
                case 0xEE: op_00EE();
                    break;
                default: unknown(opcode);
            }
            break;
        case 0x1: op_1NNN(opcode);
            break;
        case 0x2: op_2NNN(opcode);
            break;
        case 0x3: op_3XNN(opcode);
            break;
        case 0x4: op_4XNN(opcode);
            break;
        case 0x5: op_5XY0(opcode);
            break;
        case 0x6: op_6XNN(opcode);
            break;
        case 0x7: op_7XNN(opcode);
            break;
        case 0x8: op_8XY_(opcode);
            break;
        case 0x9: op_9XY0(opcode);
            break;
        case 0xA: op_ANNN(opcode);
            break;
        case 0xB: op_BNNN(opcode);
            break;
        case 0xC: op_CXNN(opcode);
            break;
        case 0xD: op_DXYN(opcode, disp);
            break;
        case 0xE: op_EX__(opcode, kb);
            break;
        case 0xF: op_FX__(opcode, kb, disp);
            break;
        default: unknown(opcode);
    }
}