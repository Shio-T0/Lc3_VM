#include <cstdint>
#include <fstream>

int main() {
  std::ofstream out("test_program.bin", std::ios::binary);

  if (!out)
    return 1;

  uint8_t program[] = {
      0x01, 0x0A, // LD A, 10
      0x02, 0x05, // ADD A, 5
      0x02, 0x03  // ADD A, 3
  };

  out.write(reinterpret_cast<char *>(program), sizeof(program));
  out.close();

  return 0;
}
