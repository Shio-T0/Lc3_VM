#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

enum {
  BR = 0,
  ADD = 1,
  LD = 2,
  ST = 3,
  JSR = 4,
  AND = 5,
  LDR = 6,
  STR = 7,
  RTI = 8,
  NOT = 9,
  LDI = 10,
  STI = 11,
  JMP = 12,
  LEA = 14,
  TRAP = 15,
};
enum traps {
  GETC = 0x20,
  OUT = 0x21,
  PUTS = 0x22,
  IN = 0x23,
  PUTSP = 0x24,
  HALT = 0x25
};

using Word = uint16_t;

std::array<Word, 8> REGISTERS;
int PC = 0;
int N, Z, P;
constexpr auto MEMORY_SIZE = 1 << (sizeof(Word) * 8);
std::vector<Word> MEMORY(MEMORY_SIZE);

struct termios initTermios;
void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &initTermios); }
void enableRawMode() {
  tcgetattr(STDIN_FILENO, &initTermios);
  atexit(disableRawMode);
  struct termios raw = initTermios;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void runBr(Word word);
void runAdd(Word word);
void runLd(Word word);
void runLdr(Word word);
void runSt(Word word);
void runStr(Word word);
void runLdi(Word word);
void runSti(Word word);
void runLea(Word word);
void runJmp(Word word);
void runJsr(Word word);
void runAnd(Word word);
void runNot(Word word);
void runRti(Word word);
void runTrap(Word word);
int main(int argc, char *argv[]) {

  for (int i = 1; i < argc; i++) {
    std::cout << "started for loop" << std::endl;
    std::fstream program(argv[i], std::ios::in | std::ios::binary);
    std::cout << "opening file" << std::endl;
    if (!program) {
      std::cerr << "Error opening file " << argv[i];
      return 1;
    };

    Word value = 0;
    while (program.read(reinterpret_cast<char *>(&value), 2)) {
      MEMORY[PC] = value;
      PC++;
    }

    PC = 0;
    program.close();
    std::cout << "file closed" << std::endl;

    // ↓ Enabing raw mode if you didnt understand already ↓
    enableRawMode();
    while (true) {
      if (PC >= MEMORY_SIZE)
        break;

      Word word = MEMORY[PC];

      switch (word >> 12) {
      case BR:
        runBr(word);
        break;
      case ADD:
        runAdd(word);
        break;
      case LD:
        runLd(word);
        break;
      case LDR:
        runLdr(word);
        break;
      case ST:
        runSt(word);
        break;
      case STR:
        runStr(word);
        break;
      case LDI:
        runLdi(word);
        break;
      case STI:
        runSti(word);
        break;
      case LEA:
        runLea(word);
        break;
      case JMP:
        runJmp(word);
        break;
      case JSR:
        runJsr(word);
        break;
      case NOT:
        runNot(word);
        break;
      case AND:
        runAnd(word);
        break;
      case RTI:
        runRti(word);
        break;
      case TRAP:
        runTrap(word);
        break;
      }
      PC++;
    }
  }
  // ↓ Disabling raw mode if you didnt understand already ↓
  disableRawMode();
}
void setcc(Word reg) {
  Word value = REGISTERS[reg];
  N = value < 0;
  Z = value == 0;
  P = value > 0;
}
void runAnd(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word left = (word & 0b0000000111000000) >> 6;
  if (word & 0b0000000000100000) {
    Word right = word & 0b0000000000011111;
    std::cout << "And-ing " << REGISTERS[left] << " with " << right
              << std::endl;
    REGISTERS[dest] = REGISTERS[left] & right;
  } else {
    Word right = word & 0b0000000000000111;
    std::cout << "And-ing " << REGISTERS[left] << " with " << REGISTERS[right]
              << std::endl;
    REGISTERS[dest] = REGISTERS[left] & REGISTERS[right];
  }

  setcc(dest);
}
void runNot(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word left = (word & 0b0000000111000000) >> 6;
  std::cout << "Not-ing " << REGISTERS[left] << std::endl;
  REGISTERS[dest] = ~REGISTERS[left];

  setcc(dest);
}

void runAdd(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word left = (word & 0b0000000111000000) >> 6;
  if (word & 0b0000000000100000) {
    Word right = word & 0b0000000000011111;
    std::cout << "Adding " << REGISTERS[left] << " + " << right << std::endl;
    REGISTERS[dest] = REGISTERS[left] + right;
  } else {
    Word right = word & 0b0000000000000111;
    std::cout << "Adding " << REGISTERS[left] << " + " << REGISTERS[right]
              << std::endl;
    REGISTERS[dest] = REGISTERS[left] + REGISTERS[right];
  }
  if (dest == 1)
    std::cout << REGISTERS[dest] << std::endl;
  setcc(dest);
}

void runBr(Word word) {
  Word offset = word & 0b0000000111111111;
  Word n = word & 0b0000100000000000;
  Word z = word & 0b0000010000000000;
  Word p = word & 0b0000001000000000;

  if (n && N) {
    PC += offset;
  } else if (z && Z) {
    PC += offset;
  } else if (p && P) {
    PC += offset;
  }
}
void runLd(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000000111111;
  std::cout << "Loading " << MEMORY[PC + offset] << " into " << REGISTERS[dest]
            << std::endl;
  REGISTERS[dest] = MEMORY[PC + offset];
  setcc(dest);
}
void runLdr(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word base = (word & 0b0000000111000000) >> 6;
  Word offset = word & 0b0000000000111111;
  std::cout << "Loading " << MEMORY[REGISTERS[base] + offset] << " into "
            << REGISTERS[dest] << std::endl;
  REGISTERS[dest] = MEMORY[REGISTERS[base] + offset];
  setcc(dest);
}
void runLdi(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000111111111;
  std::cout << "Loading " << MEMORY[MEMORY[PC + offset]] << " into "
            << REGISTERS[dest] << std::endl;
  REGISTERS[dest] = MEMORY[MEMORY[PC + offset]];
  setcc(dest);
}
void runLea(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000111111111;
  std::cout << "Loading " << PC + offset << " into " << REGISTERS[dest]
            << std::endl;
  REGISTERS[dest] = PC + offset;
}
void runSt(Word word) {
  Word source = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000111111111;
  std::cout << "Storing " << REGISTERS[source] << " into "
            << MEMORY[PC + offset] << std::endl;
  MEMORY[PC + offset] = REGISTERS[source];
}
void runStr(Word word) {
  Word source = (word & 0b0000111000000000) >> 9;
  Word base = (word & 0b0000000111000000) >> 6;
  Word offset = word & 0b0000000000111111;
  std::cout << "Storing " << REGISTERS[source] << " into "
            << MEMORY[REGISTERS[base] + offset] << std::endl;
  MEMORY[REGISTERS[base] + offset] = REGISTERS[source];
}
void runSti(Word word) {
  Word source = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000111111111;
  std::cout << "Storing " << REGISTERS[source] << " into "
            << MEMORY[MEMORY[PC + offset]] << std::endl;
  MEMORY[MEMORY[PC + offset]] = REGISTERS[source];
}
void runJmp(Word word) {
  Word base = (word & 0b0000000111000000) >> 6;
  std::cout << "Jumping to " << REGISTERS[base] << std::endl;
  PC = REGISTERS[base];
}
void runJsr(Word word) {
  Word TMP = PC;
  if (word & 0b0000100000000000) {
    Word offset = word & 0b0000011111111111;
    PC += offset;
  } else {
    Word base = (word & 0b0000000111000000) >> 6;
    std::cout << "Jumping to " << REGISTERS[base] << std::endl;
    PC = REGISTERS[base];
  }
  REGISTERS[7] = TMP;
}

void runTrap(Word word) {
  // HAHA you fell for it

  Word trapVect8 = word & 0b0000000011111111;
  switch (trapVect8) {
  case GETC: {
    char c = getchar();
    REGISTERS[0] = c;
    break;
  }
  case OUT: {
    char ch = REGISTERS[0];
    putc(ch, stdout);
    fflush(stdout);
    break;
  }
  case PUTS: {
    Word *string = &MEMORY[REGISTERS[0]];
    int i = 0;
    for (Word c = string[i]; c != 0; i++) {
      putc(c & 0x00ff, stdout);
    }
    fflush(stdout);
    break;
  }
  case IN: {
    puts(": ");
    fflush(stdout);
    char c = getchar();
    putc(c, stdout);
    fflush(stdout);
    REGISTERS[0] = c;
    break;
  }
  case PUTSP: {
    Word *string = &MEMORY[REGISTERS[0]];
    int i = 0;
    for (Word c = string[i]; c != 0; i++) {
      putc(c & 0x00ff, stdout);
      putc(c & 0xff00, stdout);
    }
    fflush(stdout);
    break;
  }
  case HALT: {
    exit(0);
    break;
  }
  }
}
