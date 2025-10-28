#pragma once

// definiç0es basicas do chip8 (tamanho da tela e configs padrao)
#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32

// endereco onde o programa começa na memoria
#define DEFAULT_PC_START 0x200

// clock padrao (instruçoes por segundo)
#define DEFAULT_CLOCK_HZ 700

// escala padrao da janela (quantos pixels reais pra cada pixel do chip8)
#define DEFAULT_SCALE 12
