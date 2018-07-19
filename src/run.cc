#include <iostream>

#include "symreg.hpp"

int main() {
  symreg::dataset ds = symreg::generate_dataset(
    [](double x) { return x*x*x; },
    100,
    0,
    200
  );

  symreg::MCTS::MCTS mcts{5, 500, ds, symreg::UCB1, symreg::NRMSD};
  mcts.iterate();

  auto res = mcts.build_result();

  std::cout << res->to_string() << std::endl;
  std::cout << symreg::NRMSD(ds, res) << std::endl;
  
  return 0;
}
