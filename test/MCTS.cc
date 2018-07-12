#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(Stuff, WhoCares) {
  symreg::dataset ds = symreg::generate_dataset(
    [](double x) { return x*x; },
    100,
    0,
    200
  );

  std::cout << ds.x.size() << std::endl;

  symreg::MCTS mcts{ds, 1};
  mcts.iterate(7);

  std::cout << mcts.to_gv() << std::endl;
  std::cout << mcts.build_result()->to_string() << std::endl;
  std::cout << mcts.build_result()->to_gv() << std::endl;
  
  ASSERT_TRUE(true);
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
