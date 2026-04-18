#include <cstdint>
#include <fstream>

int main() {
  std::ofstream out("test_program.bin", std::ios::binary);

  if (!out)
    return 1;

  uint16_t program[] = {0x2203, // LD reg1, OFFSET 3 -> 10
                        0x1265, // ADD A, 5
                        0x1263, // ADD A, 3
                        0x000A};

  out.write(reinterpret_cast<char *>(program), sizeof(program));
  out.close();

  return 0;
}
