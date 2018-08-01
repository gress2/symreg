#include <iostream>

#include "symreg.hpp"
#include "gtest/gtest.h"

TEST(MSE, IsCorrect) {
  std::shared_ptr<brick::AST::AST> ast = brick::AST::parse("_x0");
  symreg::dataset ds = symreg::generate_dataset([](int x) {
    return x + 1; 
  }, 5, 1, 6); 
  auto mse = symreg::MCTS::loss::MSE(ds, ast);
  ASSERT_TRUE(mse == 1);

  ds = symreg::generate_dataset([](int x) { return x * x; }, 5, 1, 6);
  mse = symreg::MCTS::loss::MSE(ds, ast);
  ASSERT_TRUE(mse == 116.8);
}

TEST(BindLossFn, BasicSanityCheck) {
  auto loss_fn = symreg::MCTS::loss::MSE;
  symreg::dataset ds 
    = symreg::generate_dataset([](int x) { return x; }, 5, 1, 6);
  auto bound_loss_fn = symreg::MCTS::loss::bind_loss_fn(loss_fn, ds);
  std::shared_ptr<brick::AST::AST> ast = brick::AST::parse("_x0");
  ASSERT_TRUE(bound_loss_fn(ast) == 0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
