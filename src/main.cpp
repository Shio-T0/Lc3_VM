
#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <vector>

enum { ADD = 1, LD = 2 };

using Word = uint16_t;

std::array<Word, 8> REGISTERS;
int PC = 0;
std::vector<Word> MEMORY;
constexpr auto MEMORY_SIZE = 1 << (sizeof(Word) * 8);

void runAdd(Word word);
void runLd(Word word);
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
