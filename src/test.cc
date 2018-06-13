#include "brick.hpp"
#include <iostream>

int main() {
  auto ast_ptr = brick::AST::parse("3+8");
  std::cout << ast_ptr->to_gv() << std::endl;
  return 0;
}
