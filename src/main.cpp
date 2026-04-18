#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
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

std::array<uint16_t, 8> REGISTERS;
int PC = 0;
std::vector<uint16_t> MEMORY;

void runAdd(uint16_t word);
void runLd(uint16_t word);
void runLdr(uint16_t word);
void runSt(uint16_t word);
void runStr(uint16_t word);
void runLdi(uint16_t word);
void runSti(uint16_t word);
void runLea(uint16_t word);
void runJmp(uint16_t word);
void runJsr(uint16_t word);
int main(int argc, char *argv[]) {

  for (int i = 1; i < argc; i++) {
    std::cout << "started for loop" << std::endl;
    std::fstream program(argv[i], std::ios::in | std::ios::binary);
    std::cout << "opening file" << std::endl;
    if (!program) {
      std::cerr << "Error opening file " << argv[i];
      return 1;
    };

    uint16_t value = 0;
    while (program.read(reinterpret_cast<char *>(&value), 2)) {
      MEMORY.push_back(value);
    }

    program.close();
    std::cout << "file closed" << std::endl;

    for (uint16_t word : MEMORY) {
      std::cout << word << std::endl;
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

void runAdd(uint16_t word) {
  uint16_t dest = (word & 0b0000111000000000) >> 9;
  uint16_t left = (word & 0b0000000111000000) >> 6;
  if (word & 0b0000000000100000) {
    uint16_t right = word & 0b0000000000011111;
    std::cout << "Adding " << REGISTERS[left] << " + " << right << std::endl;
    REGISTERS[dest] = REGISTERS[left] + right;
  } else {
    uint16_t right = word & 0b0000000000000111;
    std::cout << "Adding " << REGISTERS[left] << " + " << REGISTERS[right]
              << std::endl;
    REGISTERS[dest] = REGISTERS[left] + REGISTERS[right];
  }
}

void runLd(uint16_t word) {
  uint16_t dest = (word & 0b0000111000000000) >> 9;
  uint16_t offset = word & 0b0000000000111111;
  std::cout << "Loading " << MEMORY[PC + offset] << " into " << REGISTERS[dest]
            << std::endl;
  REGISTERS[dest] = MEMORY[PC + offset];
}
void runLdr(uint16_t word) {
  uint16_t dest = (word & 0b0000111000000000) >> 9;
  uint16_t base = (word & 0b0000000111000000) >> 6;
  uint16_t offset = word & 0b0000000000111111;
  std::cout << "Loading " << MEMORY[REGISTERS[base] + offset] << " into "
            << REGISTERS[dest] << std::endl;
  REGISTERS[dest] = MEMORY[REGISTERS[base] + offset];
}
void runLdi(uint16_t word) {
  uint16_t dest = (word & 0b0000111000000000) >> 9;
  uint16_t offset = word & 0b0000000111111111;
  std::cout << "Loading " << MEMORY[MEMORY[PC + offset]] << " into "
            << REGISTERS[dest] << std::endl;
  REGISTERS[dest] = MEMORY[MEMORY[PC + offset]];
}
void runLea(uint16_t word) {
  uint16_t dest = (word & 0b0000111000000000) >> 9;
  uint16_t offset = word & 0b0000000111111111;
  std::cout << "Loading " << PC + offset << " into " << REGISTERS[dest]
            << std::endl;
  REGISTERS[dest] = PC + offset;
}
void runSt(uint16_t word) {
  uint16_t source = (word & 0b0000111000000000) >> 9;
  uint16_t offset = word & 0b0000000111111111;
  std::cout << "Storing " << REGISTERS[source] << " into "
            << MEMORY[PC + offset] << std::endl;
  MEMORY[PC + offset] = REGISTERS[source];
}
void runStr(uint16_t word) {
  uint16_t source = (word & 0b0000111000000000) >> 9;
  uint16_t base = (word & 0b0000000111000000) >> 6;
  uint16_t offset = word & 0b0000000000111111;
  std::cout << "Storing " << REGISTERS[source] << " into "
            << MEMORY[REGISTERS[base] + offset] << std::endl;
  MEMORY[REGISTERS[base] + offset] = REGISTERS[source];
}
void runSti(uint16_t word) {
  uint16_t source = (word & 0b0000111000000000) >> 9;
  uint16_t offset = word & 0b0000000111111111;
  std::cout << "Storing " << REGISTERS[source] << " into "
            << MEMORY[MEMORY[PC + offset]] << std::endl;
  MEMORY[MEMORY[PC + offset]] = REGISTERS[source];
}
void runJmp(uint16_t word) {
  uint16_t base = (word & 0b0000000111000000) >> 6;
  std::cout << "Jumping to " << REGISTERS[base] << std::endl;
  PC = REGISTERS[base];
}
void runJsr(uint16_t word) {
  uint16_t TMP = PC;
  if (word & 0b0000100000000000) {
    uint16_t offset = word & 0b0000011111111111;
    PC += offset;
  } else {
    uint16_t base = (word & 0b0000000111000000) >> 6;
    std::cout << "Jumping to " << REGISTERS[base] << std::endl;
    PC = REGISTERS[base];
  }
  REGISTERS[7] = TMP;
}
