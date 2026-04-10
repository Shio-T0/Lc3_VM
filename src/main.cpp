
#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <vector>

enum { ADD = 1, LD = 2 };

std::array<uint16_t, 8> REGISTERS;
int PC = 0;
std::vector<uint16_t> MEMORY;

void runAdd(uint16_t word);
void runLd(uint16_t word);
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
