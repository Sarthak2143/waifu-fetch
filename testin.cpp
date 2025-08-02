#include <iostream>

void testOutput() {
  std::cout << "Testing characters output: " << std::endl;
  for (int i = 0; i < 5; i++) {
    std::string line(50, '#');
    std::cout << line << " (" << line.length() << ")" << std::endl;
  }
}

void testColorOutput() {
  std::cout << "Testing color output: " << std::endl;
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 50; j++) {
      printf("\033[38;2;%d;%d;%dm#\033[0m", 255 - j * 5, i * 10, 128);
    }
    printf(" Line %d\n", i);
  }
}

void testUnicode() {
  std::cout << "Direct Unicode test: " << std::endl;
  std::cout << "░▒▓█" << std::endl;

  std::cout << "Escape seq test: " << std::endl;
  std::cout << "\u2591\u2592\u2593\u2588" << std::endl;

  std::cout << "Character by character: " << std::endl;
  std::string blocks = " ░▒▓█";
  for (char c : blocks) {
    std::cout << "'" << c << "' ";
  }
  std::cout << std::endl;
}

int main(void) {
  testOutput();
  testColorOutput();
  testUnicode();
  return 0;
}
