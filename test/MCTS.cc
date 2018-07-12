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

  symreg::MCTS mcts{ds, 1, symreg::UCB1, symreg::MSE};
  mcts.iterate(8);

  auto res = mcts.build_result();

  std::cout << res->to_string() << std::endl;
  std::cout << symreg::MSE(ds, res) << std::endl;
  
  ASSERT_TRUE(true);
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
