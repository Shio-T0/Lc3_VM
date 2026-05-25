#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <endian.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdlib.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#ifdef DEBUG
#define debug(stuff) std::cerr << stuff << std::endl;
#else
#define debug(stuff) ;
#endif // DEBUG

enum mem_map_reg { KBSR = 0xfe00, KBDR = 0xfe02 };
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
using SWord = int16_t;

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
Word check_key() {
  fd_set fd;
  FD_ZERO(&fd);
  FD_SET(STDIN_FILENO, &fd);
  struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};
  return select(1, &fd, NULL, NULL, &timeout) != 0;
}

Word read_mem(Word addr) {
  if (addr == KBSR) {
    if (check_key()) {
      MEMORY[KBSR] = (1 << 15);
      MEMORY[KBDR] = getchar();
      debug("User pressed " << MEMORY[KBDR]);
    } else {
      MEMORY[KBSR] = 0;
    }
  }
  return MEMORY[addr];
};

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
// void runRti(Word word);
void runTrap(Word word);
int main(int argc, char *argv[]) {

  for (int i = 1; i < argc; i++) {
    debug("started for loop");
    std::fstream program(argv[i], std::ios::in | std::ios::binary);
    debug("opening file");
    if (!program) {
      std::cerr << "Error opening file " << argv[i];
      return 1;
    };

    Word origin;
    program.read(reinterpret_cast<char *>(&origin), 2);
    origin = be16toh(origin);
    PC = origin;
    Word value = 0;
    while (program.read(reinterpret_cast<char *>(&value), 2)) {
      if (be16toh(value) == 0xf020) {
        debug("Found it in PC=" << PC);
      }
      MEMORY[PC] = be16toh(value);
      PC++;
    }

    program.close();
    debug("file closed");

    // ↓ Enabing raw mode if you didnt understand already ↓
    enableRawMode();
    PC = origin;
    while (true) {
      if (PC >= MEMORY_SIZE)
        break;

      Word word = MEMORY[PC];
      PC++;

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
      case TRAP:
        runTrap(word);
        break;
      }
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
SWord Sext(Word word, int length) {
  SWord x = (word >> (length - 1)) & 1;
  if (x == 1) {
    word |= (0xffff << length);
  }
  return word;
}

void runAnd(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word left = (word & 0b0000000111000000) >> 6;
  if (word & 0b0000000000100000) {
    Word right = Sext(word & 0b0000000000011111, 5);
    debug("And-ing " << REGISTERS[left] << " with " << right);
    REGISTERS[dest] = REGISTERS[left] & right;
  } else {
    Word right = word & 0b0000000000000111;
    debug("And-ing " << REGISTERS[left] << " with " << REGISTERS[right]);
    REGISTERS[dest] = REGISTERS[left] & REGISTERS[right];
  }

  setcc(dest);
}
void runNot(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word left = (word & 0b0000000111000000) >> 6;
  debug("Not-ing " << REGISTERS[left]);
  REGISTERS[dest] = ~REGISTERS[left];

  setcc(dest);
}

void runAdd(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word left = (word & 0b0000000111000000) >> 6;
  if (word & 0b0000000000100000) {
    Word right = Sext(word & 0b0000000000011111, 5);
    //    debug("Adding " << REGISTERS[left] << " + " << right);
    REGISTERS[dest] = REGISTERS[left] + right;
  } else {
    Word right = word & 0b0000000000000111;
    //    debug("Adding " << REGISTERS[left] << " + " << REGISTERS[right]);
    REGISTERS[dest] = REGISTERS[left] + REGISTERS[right];
  }
  if (dest == 1)
    //    debug(REGISTERS[dest]);
    setcc(dest);
}

void runBr(Word word) {
  SWord offset = Sext(word & 0b0000000111111111, 9);
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
  SWord offset = Sext(word & 0b0000000111111111, 9);
  //  debug("LD-ing " << MEMORY[PC + offset] << " into reg" << dest);
  REGISTERS[dest] = read_mem(PC + offset);
  setcc(dest);
}
void runLdr(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word base = (word & 0b0000000111000000) >> 6;
  SWord offset = Sext(word & 0b0000000000111111, 6);
  //  debug("LDR-ing " << MEMORY[REGISTERS[base] + offset] << " into reg" <<
  //  dest);
  REGISTERS[dest] = read_mem(REGISTERS[base] + offset);
  setcc(dest);
}
void runLdi(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  SWord offset = Sext(word & 0b0000000111111111, 9);
  //  debug("LDI-ing " << MEMORY[MEMORY[PC + offset]] << " into reg" << dest);
  REGISTERS[dest] = read_mem(read_mem(PC + offset));
  setcc(dest);
}
void runLea(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  SWord offset = Sext(word & 0b0000000111111111, 9);
  //  debug("LEA-ing " << PC + offset << " into reg" << dest);
  REGISTERS[dest] = PC + offset;
}
void runSt(Word word) {
  Word source = (word & 0b0000111000000000) >> 9;
  SWord offset = Sext(word & 0b0000000111111111, 9);
  debug("ST-ing " << REGISTERS[source] << " into mem" << PC + offset);
  MEMORY[PC + offset] = REGISTERS[source];
}
void runStr(Word word) {
  Word source = (word & 0b0000111000000000) >> 9;
  Word base = (word & 0b0000000111000000) >> 6;
  SWord offset = Sext((word & 0b0000000000111111), 6);
  debug("STR-ing " << REGISTERS[source] << " into mem"
                   << REGISTERS[base] + offset);
  MEMORY[REGISTERS[base] + offset] = REGISTERS[source];
}
void runSti(Word word) {
  Word source = (word & 0b0000111000000000) >> 9;
  SWord offset = Sext(word & 0b0000000111111111, 9);
  debug("STI-ing " << REGISTERS[source] << " into mem" << MEMORY[PC + offset]);
  MEMORY[MEMORY[PC + offset]] = REGISTERS[source];
}
void runJmp(Word word) {
  Word base = (word & 0b0000000111000000) >> 6;
  debug("JMP-ing to " << REGISTERS[base]);
  PC = REGISTERS[base];
}
void runJsr(Word word) {
  Word TMP = PC;
  if (word & 0b0000100000000000) {
    SWord offset = Sext(word & 0b0000011111111111, 11);
    debug("JSR-ing to " << PC + offset);
    PC += offset;
  } else {
    Word base = (word & 0b0000000111000000) >> 6;
    debug("JSR-ing to " << REGISTERS[base]);
    PC = REGISTERS[base];
  }
  REGISTERS[7] = TMP;
}

void runTrap(Word word) {
  // HAHA you fell for it

  Word trapVect8 = word & 0b0000000011111111;
  debug("Trapping " << trapVect8);
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
    Word c = string[0];
    for (int i = 0; c != 0; i++) {
      c = string[i];
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
    Word c = string[0];
    for (int i = 0; c != 0; i++) {
      c = string[i];
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
