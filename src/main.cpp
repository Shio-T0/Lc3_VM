#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <system_error>
#include <vector>

enum {
  ADD = 1,
  LD = 2,
  ST = 3,
  JSR = 4,
  LDR = 6,
  STR = 7,
  LDI = 10,
  STI = 11,
  JMP = 12,
  LEA = 14,

};

using Word = uint16_t;

std::array<Word, 8> REGISTERS;
int PC = 0;
std::vector<Word> MEMORY;
constexpr auto MEMORY_SIZE = 1 << (sizeof(Word) * 8);
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
    MEMORY = std::vector<Word>(MEMORY_SIZE);
    while (program.read(reinterpret_cast<char *>(&value), 2)) {
      MEMORY[PC] = value;
      PC++;
    }

    PC = 0;
    program.close();
    std::cout << "file closed" << std::endl;

    while (true) {
      if (PC >= MEMORY_SIZE)
        break;

      Word word = MEMORY[PC];
      switch (word >> 12) {
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
      }
      PC++;
    }
  }
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
}

void runLd(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000000111111;
  std::cout << "Loading " << MEMORY[PC + offset] << " into " << REGISTERS[dest]
            << std::endl;
  REGISTERS[dest] = MEMORY[PC + offset];
}
void runLdr(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word base = (word & 0b0000000111000000) >> 6;
  Word offset = word & 0b0000000000111111;
  std::cout << "Loading " << MEMORY[REGISTERS[base] + offset] << " into "
            << REGISTERS[dest] << std::endl;
  REGISTERS[dest] = MEMORY[REGISTERS[base] + offset];
}
void runLdi(Word word) {
  Word dest = (word & 0b0000111000000000) >> 9;
  Word offset = word & 0b0000000111111111;
  std::cout << "Loading " << MEMORY[MEMORY[PC + offset]] << " into "
            << REGISTERS[dest] << std::endl;
  REGISTERS[dest] = MEMORY[MEMORY[PC + offset]];
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
