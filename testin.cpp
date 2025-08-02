#include <iostream>

void testOutput() {
  std::cout << "Testing characters output: " << std::endl;
  for (int i = 0; i < 5; i++) {
    std::string line(50, '#');
    std::cout << line << " (" << line.length() << ")" << std::endl;
  }
}

int main(void) {
  testOutput();
  return 0;
}
