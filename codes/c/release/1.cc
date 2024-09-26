#include <cstdlib>
#include <iostream>

void destructor() {
  std::cout << "Destructor" << std::endl;
}

struct GlobalObject {
  GlobalObject() {
    std::cout << "GlobalObject()" << std::endl;
  }
  ~GlobalObject() {
    std::cout << "~GlobalObject()" << std::endl;
  }
};

void init() {
  static GlobalObject a;
}

int main() {
  std::cout << "Main function start" << std::endl;
  init();
  std::atexit(destructor);

  std::cout << "Main function ends" << std::endl;
  return 0;
}
