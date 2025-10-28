#pragma once
#include <cstdint>
#include <string>
#include "defs.h"

// declaracoes das classes que vao ser usadas aqui
class Keyboard;
class Display;

class Chip8 {
public:
    Chip8();

    // inicia a vm, seta valores iniciais tipo pc, registradores, memoria
    void initialize(uint16_t start_pc = DEFAULT_PC_START);

    // carrega o rom pra memoria a partir de um endereco
    bool loadROM(const std::string &path, uint16_t load_addr = DEFAULT_PC_START);

    // faz um ciclo da cpu: busca, decodifica e executa uma instrucao
    void emulateCycle(Keyboard &kb, Display &disp);

    // atualiza os dois timers (delay e sound) que descem 60 vezes por segundo
    void tickTimers();

    // funcao pra pegar o estado atual da tela
    const uint8_t *video() const { return DISPLAY; }

private:
    // memoria e registradores do chip8
    uint8_t memory[4096]; // memoria total, 4kb
    uint8_t V[16]; // 16 registradores de 8 bits (v0 ate vf)
    uint16_t I; // registrador de endereco
    uint16_t PC; // program counter (endereco da proxima instrucao)
    uint8_t SP; // stack pointer (posição atual da pilha)
    uint16_t stack[16]; // pilha pra chamadas de funcao (ate 16 niveis)

    // parte da tela e timers
    uint8_t DISPLAY[CHIP8_WIDTH * CHIP8_HEIGHT]; // cada posicao eh um pixel (0 ou 1)
    uint8_t delay_timer; // timer que diminui sozinho (usado em animacoes)
    uint8_t sound_timer; // timer do som, utilizado para nao dar erro por n ter implementado

    // funcoes que tratam cada tipo de instrucao
    void op_00E0(); // limpa a tela
    void op_00EE(); // retorna de uma subrotina
    void op_1NNN(uint16_t opcode); // pula pra um endereco
    void op_2NNN(uint16_t opcode); // chama subrotina
    void op_3XNN(uint16_t opcode); // pula proxima instrucao se vx == nn
    void op_4XNN(uint16_t opcode); // pula proxima instrucao se vx != nn
    void op_5XY0(uint16_t opcode); // compara dois registradores
    void op_6XNN(uint16_t opcode); // carrega um valor em vx
    void op_7XNN(uint16_t opcode); // soma um valor em vx
    void op_8XY_(uint16_t opcode); // varias operacoes logicas/aritmeticas
    void op_9XY0(uint16_t opcode); // pula se vx != vy
    void op_ANNN(uint16_t opcode); // seta registrador I
    void op_BNNN(uint16_t opcode); // pula pra nnn + v0
    void op_CXNN(uint16_t opcode); // gera numero aleatorio e faz AND
    void op_DXYN(uint16_t opcode, Display &disp); // desenha sprite na tela
    void op_EX__(uint16_t opcode, Keyboard &kb); // instrucoes de teclado
    void op_FX__(uint16_t opcode, Keyboard &kb, Display &disp); // varias instrucoes de memoria e timer

    void unknown(uint16_t opcode) const; // chamada quando pega uma instrucao invalida
};
